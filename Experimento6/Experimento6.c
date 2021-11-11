// Aluno: Matheus Viana Coelho Albuquerque

#include <REG517A.H>

#define TAMANHO_BUFFER 16

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
	Ativa a transmissão serial, caso ainda não esteja Ativa.*/
	
	// Para inserir o byte em TxBuffer preciso saber se ele não está cheio
	if (((TxBuffer.in + 1)%TAMANHO_BUFFER) != TxBuffer.out) {
		// Caso o TxBuffer não esteja cheio
		// Insiro no buffer
		TxBuffer.buffer[TxBuffer.in] = c;
		// Incremento o TxBufferIn
		TxBuffer.in = (TxBuffer.in + 1)%TAMANHO_BUFFER;
	}
	
	// Verifica se o TxBuffer está ocupado transmitindo informação para a interface serial
	if (!txOcupado) {
		// Caso não esteja ocupado, ocupo-o e ativo a transmissão serial
		txOcupado = 1;
		TI0 = 1;
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

void baudrate_inicializa() {
	
	ADCON0 = ADCON0 | 0x80; // Faço BD = 1 para ativar o baud rate
}

void serial_inicializa() {
	S0CON = (S0CON & 0x0F) | 0x50; // Faz SM0 = 0 SM1 = 1 => ativa modo 1; SM20 = 0; REN0 = 1
	PCON = PCON | 0x80; // Ativo o bit mais significativo de PCON, basicamente SMOD = 1
}

void comunicacaoserial() interrupt 4 using 2 {
	char c;
	// Verifico tanto transmissao quanto recepção pois é full-duplex
	if (TI0) {
		TI0 = 0; // Zero o TI0 pois não é feito automaticamente
		
		if (!TxBufferVazio()) {
			// Se o tx não estiver vazio leio um caracter do TxBuffer e o envio pelo barramento serial
			c = TxBuffer.buffer[TxBuffer.out];
			TxBuffer.out = (TxBuffer.out + 1)%TAMANHO_BUFFER;
			S0BUF = c;
		}
		else 
			// Caso Tx tenha sido esvaziado, desocupo-o.
			txOcupado = 0;			
	}
	
	if (RI0) {
		RI0 = 0; // Zero o RI0 pois não é feito automaticamente
		
		c = S0BUF; // salvo o caracter lido em uma variavel auxiliar
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
	EAL = 1;	// Enable all interrupts
	ES0 = 1; 	// Enable the serial channel 0 interrupts
	initCircularBuffer();
	baudrate_inicializa();
	serial_inicializa();
	
	while(1) {
		// Verifico se chegou uma string para enviá-la para interface serial
		if (RecebeuString) {
			ReceiveString(s);
			SendString(s);
		}
	}
}