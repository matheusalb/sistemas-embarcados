#include <REG51F.H>

// Com SMOD = 1 temos TH1 ~ 203.92 ~ 204 -> Nesse caso obtive um BaudRate de 1200 bps (ESCOLHIDO!)
// Com SMOD = 0 temos TH1 ~ 229.958 ~ 230 -> Nesse caso obtive um BaudRate de 1201 bps
// TH1_RECARGA = 256 - 2^(SMOD)*12000000/(12*32*1200) <- Formula utilizada para calcular o valor de TH1 acima, leia-se ^ como operador potência 
#define TH1_RECARGA 204

bit chegouNovoByte = 0;
char byteLido;

void timer1_inicializa() {
	TR1 = 0; // Desligar timer1
	TMOD = (TMOD & 0x0F) | 0x20; // Faz gate = 0 , C/T = 0, M1 = 1 e M0 = 0 => Ativa modo 2; Observação que no caso do timer 1 utilizamos os 4 bits mais significativos para setar o TMOD com as configurações desejadas
	
	TH1 = TH1_RECARGA; // Programa valor de recarga do Timer 1.
	TL1 = TH1_RECARGA; // Faço a primeira recarga manualmente, já que a recarga ocorre quando ocorre overflow no TL1.
	ET1 = 0; // Desabilita interrupção do timer 1. (não vai ser necessário)
	TR1 = 1;  // Liga timer1.
}

void serial_inicializa() {
	SCON = (SCON & 0x0F) | 0x50; // Faz SM0 = 0 SM1 = 1 => ativa modo 1; SM2 = 0; REN = 1
	PCON = PCON | 0x80; // Ativo o bit mais significativo de PCON, basicamente SMOD = 1
}

void comunicacaoSerial() interrupt 4 using 2 {
	// Verifico tanto transmissao quanto recepção pois é full-duplex
	if (TI) {
		TI = 0; // Zero o TI pois não é feito automaticamente 
	}
	
	if (RI) {
		RI = 0; // Zero o RI pois não é feito automaticamente
		
		byteLido = SBUF; // salvo o caracter lido em uma variavel auxiliar
		chegouNovoByte = 1; // ligo a flag indicando que li um novo caracter
	}
}

void main() {
	EA = 1;		// Habilito o tratamento de interrupções
	ES = 1; 	// Habilito o tratatemento da interrupção serial
	timer1_inicializa();
	serial_inicializa();

	while (1) {
		// Verifico se chegou um novo byte
		if (chegouNovoByte) {
				chegouNovoByte = 0; // Desligo a flag
				byteLido++; // Somo um ao caractere lido
				SBUF = byteLido; // Envio o novo byte pelo barramento serial
		}
	}	
}