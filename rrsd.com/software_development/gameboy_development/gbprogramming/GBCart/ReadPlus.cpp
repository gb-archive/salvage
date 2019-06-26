void read_header(unsigned char *h)
{
	int byte, index = 0;

	ADDRESS_HIGH(0x01);

	for(byte = 0; byte < 0x50; byte++) {
		ADDRESS_LOW(byte);
		h[index++] = READ(ROM);
	}
	printf("Game name     : %s\n", &h[0x34]);

	if(h[0x46] == 0x03)
		printf("              : Super GameBoy functions included\n");

	if(h[0x4A] == 0x01)
		printf("              : Non-Japanese Game\n"); 

	if(h[0x4A] == 0x00)
		printf("              : Japanese Game\n"); 

	if(h[0x47] == 0xFF) {
		printf("Cartridge type: %d(%s)\n", h[0x47], "ROM+HuC1+RAM+BATTERY");
	    h[0x47] == 0x03; 
    }
	else {    
		printf("Cartridge type: %d(%s)\n", h[0x47], type[h[0x47]]); }
		
	printf("ROM size      : %d(%d kB)\n", h[0x48], rom[h[0x48]]);

	if(sram[h[0x47]] == MBC2)
		printf("RAM size      : 512*4 Bit\n");
	else	
		printf("RAM size      : %d(%d kB)\n", h[0x49], ram[h[0x49]]);
	checksum = ((unsigned)h[0x4E] << 8) + h[0x4F];
}

