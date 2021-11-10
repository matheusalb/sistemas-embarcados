#include <REG51F.H>

sbit P2_0 = P2^0;
sbit P2_1 = P2^1;

enum estado{MonitoraP2_0, Esperar, EsperaP2_0_ser0,
						MonitoraP2_1, EsperaP2_1_ser0};
void maquinaEstadoP2_0() {
	static char estado = EsperaP2_0_ser0;
	static int contador = 0;
	
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
			}
			break;
		case Esperar:
			// Conta 1 segundo contado via software e muda de estado.
			if (contador < 30000)
				contador++;
			else {
				contador = 0;
				P1 = P1 & 0xF0;
				estado = EsperaP2_0_ser0;
			}
			break;
	}
}

void maquinaEstadoP2_1() {
	static char estado = EsperaP2_1_ser0;
	static int contador = 0;
	
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
			}
			break;
		case Esperar:
			// Conta 1 segundo via software e muda de estado
			if (contador < 30000)
				contador++;
			else {
				contador = 0;
				P1 = P1 & 0x0F;
				estado = EsperaP2_1_ser0;
			}
			break;
	}
}

void main() {
	while (1) {
		maquinaEstadoP2_0();
		maquinaEstadoP2_1();
	}
}