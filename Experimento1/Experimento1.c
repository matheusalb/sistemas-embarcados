#include <REG51F.H>

void main() {
	while(1) {
		
		// Verificando se o bit0 de P2 � 1
		if((P2 & 0x01) == 0x01) 
			// Atribui��o dos 4 bits menos significados de P0 aos 4 bits menos significativos de P1
			P1 = (P1 | 0x0F) & (P0 | 0xF0);
		else
			// Sen�o, atribuo zero aos 4 bits menos significativos de P1
			P1 = P1 & 0xF0;
	
		// Verificando se o bit1 de P2 == 1
		if((P2 & 0x02) == 0x02)
			// Atribuo os 4 bits mais significativos de P0 aos 4 bits mais significativos de P1
			P1 = (P1 | 0xF0) & (P0 | 0x0F);
		else
			// Sen�o, atribuo zero aos 4 bits mais significativos de P1.
			P1 = P1 & 0x0F;
	}
}