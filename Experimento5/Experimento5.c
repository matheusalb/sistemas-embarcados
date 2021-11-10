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
	/*Fun��o que recebe o byte �c� e insere em TxBuffer.
	Ativa a transmiss�o serial, caso ainda n�o esteja ativa.*/
	
	// Para inserir o byte em TxBuffer preciso saber se ele n�o est� cheio
	if (((TxBuffer.in + 1)%TAMANHO_BUFFER) != TxBuffer.out) {
		// Caso o TxBuffer n�o esteja cheio
		// Insiro no buffer
		TxBuffer.buffer[TxBuffer.in] = c;
		// Incremento o TxBufferIn
		TxBuffer.in = (TxBuffer.in + 1)%TAMANHO_BUFFER;
	}
	
	// Verifica se o TxBuffer est� ocupando transmitindo informa��o para a interface serial
	if (!txOcupado) {
		// Caso n�o esteja ocupado, ocupo-o e ativo a transmiss�o serial
		txOcupado = 1;
		TI = 1;
	}
		
}

void SendString(char *s) {
	//Fun��o que insere a string apontada por �*s� em TxBuffer usando a fun��o SendChar(c).
	
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
	// Fun��o que retorna 1 caso o buffer circular de recep��o esteja vazio e 0 em caso contr�rio.
	char vazio = 0;
	if (RxBuffer.in == RxBuffer.out)
		vazio = 1;
	return vazio;
}

char TxBufferVazio() {
	// Fun��o que retorna 1 caso o buffer circular de transmissao esteja vazio e 0 em caso contr�rio.
	char vazio = 0;
	if (TxBuffer.in == TxBuffer.out)
		vazio = 1;
	return vazio;
}

char ReceiveChar() {
	// Fun��o que retorna um byte de RxBuffer
	
	char character = 0;
	// Verifico se o RxBuffer n�o est� vazio para retornar um caracter
	if (!RxBufferVazio()) {
		character = RxBuffer.buffer[RxBuffer.out];
		RxBuffer.out = (RxBuffer.out + 1)%TAMANHO_BUFFER; 
	}
	return character;
}


void ReceiveString(char *s) {
	// Fun��o que copia uma string de RxBuffer para o local apontado por �*s�, usando a fun��o ReceiveChar().
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
	TMOD = (TMOD & 0x0F) | 0x20; // Faz gate = 0 , C/T = 0, M1 = 1 e M0 = 0 => Ativa modo 2; Observa��o que no caso do timer 1 utilizamos os 4 bits mais significativos para setar o TMOD com as configura��es desejadas
	
	TH1 = TH1_RECARGA; // Programa valor de recarga do Timer 1.
	//TL1 = TH1_RECARGA; // Fa�o a primeira recarga manualmente, j� que a recarga ocorre quando ocorre overflow no TL1.
	ET1 = 0; // Desabilita interrup��o do timer 1. (n�o vai ser necess�rio)
	TR1 = 1;  // Liga timer1.
}

void serial_inicializa() {
	SCON = (SCON & 0x0F) | 0x50; // Faz SM0 = 0 SM1 = 1 => ativa modo 1; SM2 = 0; REN = 1
	PCON = PCON | 0x80; // Ativo o bit mais significativo de PCON, basicamente SMOD = 1
}

void comunicacaoSerial() interrupt 4 using 2 {
	char c;
	// Verifico tanto transmissao quanto recep��o pois � full-duplex
	if (TI) {
		TI = 0; // Zero o TI pois n�o � feito automaticamente
		
		if (!TxBufferVazio()) {
			// Se o tx n�o estiver vazio leio um caracter do TxBuffer e o envio pelo barramento serial
			c = TxBuffer.buffer[TxBuffer.out];
			TxBuffer.out = (TxBuffer.out + 1)%TAMANHO_BUFFER;
			SBUF = c;
		}
		else 
			// Caso Tx tanha sido esvaziado, desocupo-o.
			txOcupado = 0;			
	}
	
	if (RI) {
		RI = 0; // Zero o RI pois n�o � feito automaticamente
		
		c = SBUF; // salvo o caracter lido em uma variavel auxiliar
		// Verifico se o RxBuffer n�o est� cheio para inserir o caracter nele
		if (((RxBuffer.in + 1)%TAMANHO_BUFFER) != RxBuffer.out) {
			// Caso o RxBuffer n�o esteja cheio
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
	EA = 1;		// Habilito o tratamento de interrup��es
	ES = 1; 	// Habilito o tratamento da interrup��o serial
	initCircularBuffer();
	timer1_inicializa();
	serial_inicializa();
	
	while(1) {
		// Verifico se chegou uma string para envi�-la para interface serial
		if (RecebeuString) {
			ReceiveString(s);
			SendString(s);
		}
	}
}