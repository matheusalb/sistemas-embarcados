#include <REG51F.H>

#define CORRECAO 10
#define FrClk 12000000
#define FreqTimer0_emHz 100
#define TH0_Inicial ((65536-(FrClk/(12*FreqTimer0_emHz))+CORRECAO)>>8)
#define TL0_Inicial ((65536-(FrClk/(12*FreqTimer0_emHz))+CORRECAO)&0xFF)

sbit P2_0 = P2^0;
sbit P2_1 = P2^1;

unsigned char contador_P2_0 = 0;
unsigned char contador_P2_1 = 0;

enum estado{MonitoraP2_0, Esperar, EsperaP2_0_ser0,
						MonitoraP2_1, EsperaP2_1_ser0};
	
void maquinaEstadoP2_0() {
	static char estado = EsperaP2_0_ser0;
	
	switch (estado) {
		case EsperaP2_0_ser0:
			// Espera o P2.0 ser 0
			if (P2_0 == 0) 
				estado = MonitoraP2_0;
			break;
		case MonitoraP2_0:
			// Verifica se houve transição de 0 para 1 de P2.0
			if (P2_0 == 1) {
				P1 = (P1 & 0xF0) | (P0 & 0x0F);
				estado = Esperar;
				contador_P2_0 = 0;
			}
			break;
		case Esperar:
			// Espera 1 segundo. (100 * 10ms = 1 segundo)
			if (contador_P2_0 >= 100) {
				P1 = P1 & 0xF0;
				estado = EsperaP2_0_ser0;
			}
			
			break;
	}
}

void maquinaEstadoP2_1() {
	static char estado = EsperaP2_1_ser0;
	
	switch(estado) {
		case EsperaP2_1_ser0:
			// Espera P2.1 ser 0
			if (P2_1 == 0)
				estado = MonitoraP2_1;
			break;
		case MonitoraP2_1:
			// Verifica se houve transição de 0 para 1 de P2.1
			if (P2_1 == 1) {
				P1 = (P1 & 0x0F) | (P0 & 0xF0);
				estado = Esperar;
				// Zero o contador para contar novamente 1 segundo.
				contador_P2_1 = 0;
			}
			break;
		case Esperar:
			// Espera 1 segundo. (100 * 10ms = 1 segundo)
			if (contador_P2_1 >= 100) {
				P1 = P1 & 0x0F;
				estado = EsperaP2_1_ser0;
			}
			break;
	}
}

void timer0_inicializa() {
	TR0 = 0; // Desligar timer0
	TMOD = (TMOD & 0xF0) | 0x01; // Faz gate = 0 , C/T = 0, M1 = 0 e M0 = 1 => Ativa modo 1.
	TH0 = TH0_Inicial; // Programa valor de contagem do Timer0.
	TL0 = TL0_Inicial;
	ET0 = 1; // Habilita interrupção do timer 0.
	TR0 = 1;  // Liga timer0.
}

void timer0_int(void) interrupt 1 using 2 {
	TR0 = 0; // Desliga Timer0
	TL0 += TL0_Inicial; // Faz recarga da contagem do timer0
	TH0 += TH0_Inicial + (unsigned char) CY;
	TR0 = 1; //Habilita contagem do timer0
	
	// Instruções acima demoram 0.000010 segundos, e como o 12Mhz/12 = 1Mhz, fazendo 1Mhz * 0.000010s temos um fator de correção de 10.
	
	// Incrementa contadores
	contador_P2_0++;
	contador_P2_1++;
}

void main() {
	// Inicializando o nosso timer0
	timer0_inicializa();
	EA = 1; // Habilitando o tratamento de interrupções.
	while (1) {
		maquinaEstadoP2_0();
		maquinaEstadoP2_1();
	}
}