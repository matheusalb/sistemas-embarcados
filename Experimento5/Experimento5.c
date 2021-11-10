// Aluno: Matheus Viana Coelho Albuquerque

#include <REG51F.H>

#define TAMANHO_BUFFER 16
#define TH1_RECARGA 204

// Estrutura para representar o buffer circular
typedef struct {
	char buffer[TAMANHO_BUFFER];
	unsigned char in;
	unsigned char out;
}CircularBuffer;

CircularBuffer TxBuffer;
CircularBuffer RxBuffer;
bit RecebeuString = 0;
bit txOcupado = 0;

void SendChar(char c) {
	/*Função que recebe o byte “c” e insere em TxBuffer.
	Ativa a transmissão serial, caso ainda não esteja ativa.*/
	
	// Para inserir o byte em TxBuffer preciso saber se ele não está cheio
	if (((TxBuffer.in + 1)%TAMANHO_BUFFER) != TxBuffer.out) {
		// Caso o TxBuffer não esteja cheio
		// Insiro no buffer
		TxBuffer.buffer[TxBuffer.in] = c;
		// Incremento o TxBufferIn
		TxBuffer.in = (TxBuffer.in + 1)%TAMANHO_BUFFER;
	}
	
	// Verifica se o TxBuffer está ocupando transmitindo informação para a interface serial
	if (!txOcupado) {
		// Caso não esteja ocupado, ocupo-o e ativo a transmissão serial
		txOcupado = 1;
		TI = 1;
	}
		
}

void SendString(char *s) {
	//Função que insere a string apontada por “*s” em TxBuffer usando a função SendChar(c).
	
	int i = 0;
	char curCharacter;
	// Loop que envia os caracteres da string para TxBuffer por meio de SendChar
	do{
		curCharacter = s[i];
		SendChar(curCharacter);
		i++;
	}while(curCharacter != '$');
}

char RxBufferVazio() {
	// Função que retorna 1 caso o buffer circular de recepção esteja vazio e 0 em caso contrário.
	char vazio = 0;
	if (RxBuffer.in == RxBuffer.out)
		vazio = 1;
	return vazio;
}

char TxBufferVazio() {
	// Função que retorna 1 caso o buffer circular de transmissao esteja vazio e 0 em caso contrário.
	char vazio = 0;
	if (TxBuffer.in == TxBuffer.out)
		vazio = 1;
	return vazio;
}

char ReceiveChar() {
	// Função que retorna um byte de RxBuffer
	
	char character = 0;
	// Verifico se o RxBuffer não está vazio para retornar um caracter
	if (!RxBufferVazio()) {
		character = RxBuffer.buffer[RxBuffer.out];
		RxBuffer.out = (RxBuffer.out + 1)%TAMANHO_BUFFER; 
	}
	return character;
}


void ReceiveString(char *s) {
	// Função que copia uma string de RxBuffer para o local apontado por “*s”, usando a função ReceiveChar().
	char character;
	int i = 0;
	do {
		// Loop para ler o caracter do RxBuffer e adiconar a string, termina quando encontra o $ indicando o final da string
		character = ReceiveChar();
		s[i] = character;
		i++;
	} while(character != '$' && !RxBufferVazio());
	
	RecebeuString = 0;
}	

void timer1_inicializa() {
	TR1 = 0; // Desligar timer1
	TMOD = (TMOD & 0x0F) | 0x20; // Faz gate = 0 , C/T = 0, M1 = 1 e M0 = 0 => Ativa modo 2; Observação que no caso do timer 1 utilizamos os 4 bits mais significativos para setar o TMOD com as configurações desejadas
	
	TH1 = TH1_RECARGA; // Programa valor de recarga do Timer 1.
	//TL1 = TH1_RECARGA; // Faço a primeira recarga manualmente, já que a recarga ocorre quando ocorre overflow no TL1.
	ET1 = 0; // Desabilita interrupção do timer 1. (não vai ser necessário)
	TR1 = 1;  // Liga timer1.
}

void serial_inicializa() {
	SCON = (SCON & 0x0F) | 0x50; // Faz SM0 = 0 SM1 = 1 => ativa modo 1; SM2 = 0; REN = 1
	PCON = PCON | 0x80; // Ativo o bit mais significativo de PCON, basicamente SMOD = 1
}

void comunicacaoSerial() interrupt 4 using 2 {
	char c;
	// Verifico tanto transmissao quanto recepção pois é full-duplex
	if (TI) {
		TI = 0; // Zero o TI pois não é feito automaticamente
		
		if (!TxBufferVazio()) {
			// Se o tx não estiver vazio leio um caracter do TxBuffer e o envio pelo barramento serial
			c = TxBuffer.buffer[TxBuffer.out];
			TxBuffer.out = (TxBuffer.out + 1)%TAMANHO_BUFFER;
			SBUF = c;
		}
		else 
			// Caso Tx tanha sido esvaziado, desocupo-o.
			txOcupado = 0;			
	}
	
	if (RI) {
		RI = 0; // Zero o RI pois não é feito automaticamente
		
		c = SBUF; // salvo o caracter lido em uma variavel auxiliar
		// Verifico se o RxBuffer não está cheio para inserir o caracter nele
		if (((RxBuffer.in + 1)%TAMANHO_BUFFER) != RxBuffer.out) {
			// Caso o RxBuffer não esteja cheio
			// Insiro no buffer
			RxBuffer.buffer[RxBuffer.in] = c;
			// Incremento o TxBufferIn
			RxBuffer.in = (RxBuffer.in + 1)%TAMANHO_BUFFER;
		}
		if (c == '$')
				RecebeuString = 1;
	}
}

void initCircularBuffer() {
	TxBuffer.in = 0;
	TxBuffer.out = 0;
	RxBuffer.in = 0;
	RxBuffer.out = 0;
}

void main() {
	char s[TAMANHO_BUFFER+1];
	EA = 1;		// Habilito o tratamento de interrupções
	ES = 1; 	// Habilito o tratamento da interrupção serial
	initCircularBuffer();
	timer1_inicializa();
	serial_inicializa();
	
	while(1) {
		// Verifico se chegou uma string para enviá-la para interface serial
		if (RecebeuString) {
			ReceiveString(s);
			SendString(s);
		}
	}
}