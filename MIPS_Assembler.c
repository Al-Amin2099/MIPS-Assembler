//C program for CSUF student group project EGPC 520
//MIPS assembler
//
//Stewart Watkins
//Tony Amin
//Kevin Hart

#include <stdio.h>	//for fscanf/printf
#include <stdlib.h>	//for malloc
#include <string.h>	//for strcpy/strncmp

//global variables
FILE *asmFile, *objFile;	//file pointers
int PC = 0x00000000;		//program counter

struct Core_Instruction
{
	char instruction[10];
	char opcode[10];//acts as funct for R-type
	//char *instruction;
	//char *opcode;//acts as funct for R-type
}; 

struct Arithmetic_Core_Instruction
{
	char instruction[10];
	char fmt[10];
	char funct[10];

	//char *instruction;
	//char *fmt;
	//char *funct;
};

struct Register
{
	char name[10];
	char number[10];

	//char *name;
	//char *number;
};

struct Symbol_Table
{
	char *label;
	//char label[20];
	unsigned int address;
};

struct Core_Instruction Instruction_Set_R[] = //opcode for R type is '000000', these are funct values
{
	{"add", "100000"}, {"addu", "100001"}, {"and", "100100"}, {"jr", "001000"}, {"nor", "100111"},
	{"or", "100101"}, {"slt", "101010"}, {"sltu", "101011"}, {"sll", "000000"}, {"srl", "000010"},
	{"sub", "100010"}, {"subu", "100011"}, {"div", "011010"}, {"divu", "011011"}, {"mfhi", "010000"},
	{"mflo", "010010"}, {"mfc0", "010000"}, {"mult", "011000"}, {"multu", "011001"}, {"sra", "000011"} 
};//mfc0 value is opcode not funct
		
struct Core_Instruction Instruction_Set_I[] = 
{
	{"addi", "001000"}, {"addiu", "001001"}, {"andi", "001100"}, {"beq", "000100"}, {"bne", "000101"},
	{"lbu", "100100"}, {"lhu", "100101"}, {"ll", "110000"}, {"lui", "001111"}, {"lw", "100011"},
	{"ori", "001101"}, {"slti", "001010"}, {"sltiu", "001011"}, {"sb", "101000"}, {"sc", "111000"},
	{"sh", "101001"}, {"sw", "101011"}, {"lwcl", "110001"}, {"ldcl", "110101"}, {"swcl", "111001"},
	{"sdcl", "111101"}

};

struct Core_Instruction Instruction_Set_J[] = 
{ 
	{"j", "000010"}, {"jal", "000011"}
};

struct Arithmetic_Core_Instruction Instruction_Set_FR[] = //opcode for FR type is '010001'
{
	{"add.s", "10000", "000000"}, {"add.d", "10001", "000000"}, {"c.eq.s", "10000", "110010"},
	{"c.eq.d", "10001", "110010"}, {"c.lt.s", "10000", "111100"}, {"c.lt.d", "10001", "111100"},
	{"c.le.s", "10000", "111110"}, {"c.le.d", "10001", "111110"}, {"div.s", "10000", "000011"},
	{"div.d", "10001", "000011"}, {"mul.s", "10000", "000010"}, {"mul.d", "10001", "000010"},
	{"sub.s", "10000", "000001"}, {"sub.d", "10001", "000001"}
};

struct Arithmetic_Core_Instruction Instruction_Set_FI[] = //opcode for FI type is '010001'
{
	{"bclt", "01000", "00001"}, {"bclf", "01000", "00000"}
};

struct Register Integer_Registers[] =
{
	{"$zero", "00000"}, {"$at", "00001"}, {"$v0", "00010"}, {"$v1", "00011"}, {"$a0", "00100"}, {"$a1", "00101"}, {"$a2", "00110"}, {"$a3", "00111"},
	{"$t0", "01000"}, {"$t1", "01001"}, {"$t2", "01010"}, {"$t3", "01011"}, {"$t4", "01100"}, {"$t5", "01101"}, {"$t6", "01110"}, {"$t7", "01111"},
	{"$s0", "10000"}, {"$s1", "10001"}, {"$s2", "10010"}, {"$s3", "10011"}, {"$s4", "10100"}, {"$s5", "10101"}, {"$s6", "10110"}, {"$s7", "10111"},
	{"$t8", "11000"}, {"$t9", "11001"}, {"$k0", "11010"}, {"$k1", "11011"}, {"$gp", "11100"}, {"$sp", "11101"}, {"$fp", "11110"}, {"$ra", "11111"}
};

struct Register Float_Registers[] =
{
	{"$f0", "00000"}, {"$f1", "00001"}, {"$f2", "00010"}, {"$f3", "00011"}, {"$f4", "00100"}, {"$f5", "00101"}, {"$f6", "00110"}, {"$f7", "00111"},
	{"$f8", "01000"}, {"$f9", "01001"}, {"$f10", "01010"}, {"$f11", "01011"}, {"$f12", "01100"}, {"$f13", "01101"}, {"$f14", "01110"}, {"$f15", "01111"},
	{"$f16", "10000"}, {"$f17", "10001"}, {"$f18", "10010"}, {"$f19", "10011"}, {"$f20", "10100"}, {"$f21", "10101"}, {"$f22", "10110"}, {"$f23", "10111"},
	{"$f24", "11000"}, {"$f25", "11001"}, {"$f26", "11010"}, {"$f27", "11011"}, {"$f28", "11100"}, {"$f29", "11101"}, {"$f30", "11110"}, {"$f31", "11111"}
};

struct Symbol_Table Symbol_Table[10]; //will get populated during first pass

//helper functions
//pass one helpers
void Put_Symbol_Table(char* ptr, int pc)
{
	static int label_index = 0;
	char* label = (char*)malloc(100);
	strcpy(label, ptr); //needed to modify values

	int i = 0;
	while (label[i])
	{
		if (label[i] == ':')
		{
			label[i] = 0;
			Symbol_Table[label_index].label = label;
			Symbol_Table[label_index].address = pc;
			label_index++;
			break;
		}
		i++;
	}
}

//pass two helpers
void Remove_Char(char *ptr, char ch)
{
	while (*ptr)            //ptr is string
	{                       //ch is character to be removed from string
		if (*ptr == ch)
		{
			*ptr = 0;
		}
		ptr++;
	}
}

char *Get_Int_Reg_Num(char *ptr)
{
	char *s;
	for (int i = 0; i < (sizeof(Integer_Registers) / sizeof(Integer_Registers[0])); i++)
	{
		if (strncmp(ptr, Integer_Registers[i].name, sizeof(Integer_Registers[i].name)) == 0)
		{
			s = Integer_Registers[i].number;
			break;
		}
	}
	return s;
}

char *Get_FP_Reg_Num(char *ptr)
{
	char *s;
	for (int i = 0; i < (sizeof(Float_Registers) / sizeof(Float_Registers[0])); i++)
	{
		if (strncmp(ptr, Float_Registers[i].name, sizeof(Float_Registers[i].name)) == 0)
		{
			s = Float_Registers[i].number;
			break;
		}
	}
	return s;
}

int Get_Label_Line_Number(char *ptr)
{
	int x;
	for (int i = 0; i < (sizeof(Symbol_Table) / sizeof(Symbol_Table[0])); i++)
	{
		if (strncmp(ptr, Symbol_Table[i].label, strlen(Symbol_Table[i].label)) == 0)
		{
			x = Symbol_Table[i].address;
			break;
		}
	}
	return x;
}

char *Dec_To_SignExt_Bin(unsigned int x)	//always returns 16 bit number
{
	char bin[29] = {0}; //always assume 28 bit number + null char (for largest imm J type)
						//J type takes the first 28, cuts off the last 2
	for (int i = 27; i >= 0; i--)
	{
		bin[i] = (x & 0x1) + '0'; //place bits in reverse index for correct order
		x = x >> 1;
	}

	char *out = (char*)malloc(sizeof(bin));	//returnable pointer
	strcpy(out, bin);
	return out;
}

//processing functions
void Process_R_Instr(int index)
{

	//initialize values
	char opcode[] = "000000";
	char* rs = "00000";
	char* rt = "00000";
	char* rd = "00000";
	char *shamt = "00000";
	//set values
	char funct[7];
	sscanf(Instruction_Set_R[index].opcode, "%06s", funct);

	if (index == 0 || index == 1 ||	//check for format of instr
		index == 2 || index == 4 ||
		index == 5 || index == 6 ||
		index == 7 || index == 10 ||
		index == 11)	//instruction uses rd, rs, rt
	{	//ingest the rest of the line of code
		fscanf(asmFile, "%s", rd);	
		fscanf(asmFile, "%s", rs);
		fscanf(asmFile, "%s", rt);
		//sanitize strings
		Remove_Char(rd, ',');
		Remove_Char(rs, ',');
		Remove_Char(rt, ',');
		//turn into binary
		rd = Get_Int_Reg_Num(rd);
		rs = Get_Int_Reg_Num(rs);
		rt = Get_Int_Reg_Num(rt);
		shamt = "00000";			//even though rd/rs/rt/shamt get initialized with values at the beginning of the funct,
									//they seem to maintain the values from previous passes through the if statement,
									//need to be set again -inside- the if statement to work
	}
	else if (index == 12 || index == 13 ||
			 index == 17 || index == 18)	//instruction uses rs, rt
	{
		fscanf(asmFile, "%s", rs);
		fscanf(asmFile, "%s", rt);

		Remove_Char(rs, ',');
		Remove_Char(rt, ',');

		rd = "00000";
		rs = Get_Int_Reg_Num(rs);
		rt = Get_Int_Reg_Num(rt);
		shamt = "00000";
	}
	else if (index == 16)	//mfc0 instruction uses rd, rs, func is 0, opcode is 0x10
	{
		//swap func and opcode values for mfc0
		char tmp[10];
		strcpy(tmp, funct);
		strcpy(funct, opcode);
		strcpy(opcode, tmp);

		fscanf(asmFile, "%s", rd);
		fscanf(asmFile, "%s", rs);

		Remove_Char(rd, ',');
		Remove_Char(rs, ',');

		rd = Get_Int_Reg_Num(rd);
		rs = Get_Int_Reg_Num(rs);
		rt = "00000";
		shamt = "00000";
	}
	else if (index == 3)	//jr instruction uses rs, direct addressing
	{
		fscanf(asmFile, "%s", rs);

		Remove_Char(rs, ',');

		rt = "00000";
		rd = "00000";
		rs = Get_Int_Reg_Num(rs);
		shamt = "00000";
	}
	else if (index == 14 || index == 15)	//mfhi & mflo use rd
	{
		fscanf(asmFile, "%s", rd);

		Remove_Char(rd, ',');

		rd = Get_Int_Reg_Num(rd);
		rs = "00000";
		rt = "00000";
		shamt = "00000";
	}
	else if (index == 8 || index == 9 ||
			 index == 19)	//shift functions use rd, rt and shamt
	{
		int tmp;
		fscanf(asmFile, "%s", rd);
		fscanf(asmFile, "%s", rt);
		fscanf(asmFile, "%d", &tmp);

		Remove_Char(rd, ',');
		Remove_Char(rt, ',');

		rd = Get_Int_Reg_Num(rd);
		rt = Get_Int_Reg_Num(rt);
		rs = "00000";
		shamt = Dec_To_SignExt_Bin(tmp);
		shamt += 23;			//Dec_To_SignExt_Bin returns 28-bit value, increment shamt ptr by 23 to get required 5-bit value
	}
	else
	{
		printf("error in %s", __func__);
	}

	//form instruction string
	printf("individual: op:%s, rs:%s, rt:%s, rd:%s, shamt:%s, funct:%s\n", opcode, rs, rt, rd, shamt, funct);
	char binary_instr[32];
	strcpy(binary_instr, opcode);
	strcat(binary_instr, rs);
	strcat(binary_instr, rt);
	strcat(binary_instr, rd);
	strcat(binary_instr, shamt);
	strcat(binary_instr, funct);

	
	//write to file
	fprintf(objFile, "%s\n", binary_instr);
	//fprintf(objFile, "%s	%s\n", binary_instr, Instruction_Set_R[index].instruction);
	//printf("wrote %s\n", binary_instr);
}

void Process_I_Instr(int index)
{
	//initialize values
	char opcode[] = "000000";
	sscanf(Instruction_Set_I[index].opcode, "%06s", opcode);
	char *rs = "00000";
	char *rt = "00000";
	char *imm = "0000000000000000"; //16 bits
	
	if (index == 0  || index == 1 ||	//rt, rs, signextimm "addi/addiu rt, rs, imm" immediate addressing
		index == 11 || index == 12)
	{
		int tmp;
		fscanf(asmFile, "%s", rt);//ingest the rest of the line of code
		fscanf(asmFile, "%s", rs);	
		fscanf(asmFile, "%d", &tmp);

		Remove_Char(rt, ',');
		Remove_Char(rs, ',');

		rt = Get_Int_Reg_Num(rt);
		rs = Get_Int_Reg_Num(rs);
		imm = Dec_To_SignExt_Bin(tmp);
		imm += 12; //Dec_To_SignExt_Bin returns 28-bit value, increment imm ptr by 12 to get required 16-bit value
	}
	else if (index == 5  || index == 6  ||	//rt, rs, signextimm "lbu/lhu/ll rt, imm(rs)" base addressing
			 index == 7  || index == 9  ||
			 index == 13 || index == 14 ||
			 index == 15 || index == 16)
	{
		int tmp;
		fscanf(asmFile, "%s", rt);	//ingest the rest of the line of code
		fscanf(asmFile, "%d", &tmp);
		fscanf(asmFile, "%s", rs);	//"(rs)" parenthesis need removing

		Remove_Char(rt, ',');
		if (*rs = '(')
		{
			rs++;
		}
		Remove_Char(rs, ')');

		rt = Get_Int_Reg_Num(rt);
		rs = Get_Int_Reg_Num(rs);
		imm = Dec_To_SignExt_Bin(tmp);
		imm += 12; //Dec_To_SignExt_Bin returns 28-bit value, increment imm ptr by 12 to get required 16-bit value
	}
	else if (index == 17 || index == 18 || //rt, rs, signextimm "lwcl ft, imm(rs) fp version of load rt = fp reg
			 index == 19 || index == 20)
	{
		int tmp;
		fscanf(asmFile, "%s", rt);	//rt is an fp reg here
		fscanf(asmFile, "%d", &tmp);
		fscanf(asmFile, "%s", rs);	//"(rs)" parenthesis need removing

		Remove_Char(rt, ',');
		if (*rs = '(')
		{
			rs++;
		}
		Remove_Char(rs, ')');

		rt = Get_FP_Reg_Num(rt);	//rt is an fp reg
		rs = Get_Int_Reg_Num(rs);
		imm = Dec_To_SignExt_Bin(tmp);
		imm += 12; //Dec_To_SignExt_Bin returns 28-bit value, increment imm ptr by 12 to get required 16-bit value
	}
	else if (index == 2 || index == 10)	//rt, rs, zeroextimm "andi/ori rt, rs, imm" immediate addressing
	{
		unsigned int tmp;
		fscanf(asmFile, "%s", rt);	//ingest the rest of the line of code
		fscanf(asmFile, "%s", rs);
		fscanf(asmFile, "%u", &tmp); //forcing imm as unsigned int
									 //this way imm works with Dec_To_SignExt_Bin for zero extending (probably works either way)
		Remove_Char(rt, ',');
		Remove_Char(rs, ',');

		rt = Get_Int_Reg_Num(rt);
		rs = Get_Int_Reg_Num(rs);
		imm = Dec_To_SignExt_Bin(tmp);
		imm += 12; //Dec_To_SignExt_Bin returns 28-bit value, increment imm ptr by 12 to get required 16-bit value
	}
	else if (index == 3 || index == 4)	//rs, rt, branchaddr "beq rs, rt, label" PC relative addressing, sign extended
	{
		int tmp;
		fscanf(asmFile, "%s", rs);	//ingest the rest of the line of code
		fscanf(asmFile, "%s", rt);
		fscanf(asmFile, "%s", imm);	//label here

		Remove_Char(rs, ',');
		Remove_Char(rt, ',');

		rt = Get_Int_Reg_Num(rt);
		rs = Get_Int_Reg_Num(rs);
		tmp = Get_Label_Line_Number(imm);	//label -> number -> binary string
		tmp = tmp - (PC + 0x04);				//pc relative addressing, imm = addr - PC + 4
		imm = Dec_To_SignExt_Bin(tmp);		
		imm += 12; //Dec_To_SignExt_Bin returns 28-bit value, increment imm ptr by 12 to get required 16-bit value
	}
	else if (index == 8)	//rt, imm "lui rt, imm" immediate addressing
	{
		int tmp;
		fscanf(asmFile, "%s", rt);
		fscanf(asmFile, "%d", &tmp);	//label here

		Remove_Char(rt, ',');

		rt = Get_Int_Reg_Num(rt);
		rs = "00000";
		imm = Dec_To_SignExt_Bin(tmp);
		imm += 12; //Dec_To_SignExt_Bin returns 28-bit value, increment imm ptr by 12 to get required 16-bit value
	}
	else
	{
		printf("error in %s", __func__);
	}

	//form instruction string
	printf("individual: op:%s, rs:%s, rt:%s, imm:%s\n", opcode, rs, rt, imm);
	char binary_instr[32];
	strcpy(binary_instr, opcode);
	strcat(binary_instr, rs);
	strcat(binary_instr, rt);
	strcat(binary_instr, imm);

	//write to file
	fprintf(objFile, "%s\n", binary_instr);
	//fprintf(objFile, "%s	%s\n", binary_instr, Instruction_Set_I[index].instruction);
	//printf("wrote %s\n", binary_instr);
}

void Process_J_Instr(int index)
{
	//initialize values
	char opcode[] = "000000";
	sscanf(Instruction_Set_J[index].opcode, "%06s", opcode); //"j/jal label"
	char* addr = "0000000000000000000000000000";	//28 chars for bits
									//ingest remainder of line of code
	fscanf(asmFile, "%s", addr);	//both instructions have the same format
	int tmp;
									//label -> number -> binary string
	tmp = Get_Label_Line_Number(addr);	//32 bit label psedo direct addressing, take off 2 lsb and 4 msb

	strcpy(addr, Dec_To_SignExt_Bin(tmp));	//Dec_To_SignExt_Bin returns first 28 (this removes 4 msb)
	addr[26] = '\0';						//cut off 2 lsb, addr is now 26 bits
	
	//form instruction string
	printf("individual: op:%s, addr:%s\n", opcode, addr);
	char binary_instr[32];
	strcpy(binary_instr, opcode);
	strcat(binary_instr, addr);

	//write to file
	fprintf(objFile, "%s\n", binary_instr);
	//fprintf(objFile, "%s	%s\n", binary_instr, Instruction_Set_J[index].instruction);
	//printf("wrote %s\n", binary_instr);
}

void Process_FR_Instr(int index)
{
	//initialize values
	char opcode[] = "010001";	//0x11 FR
	char *ft = "00000";
	char *fs = "00000";
	char *fd = "00000";
	char *fmt = Instruction_Set_FR[index].fmt;
	char *funct = Instruction_Set_FR[index].funct;

	if (index == 0  || index == 1  ||	//fd, fs, ft "add.s fd, fs, ft"
		index == 8  || index == 9  || 
		index == 10 || index == 11 || 
		index == 12 || index == 13)
	{
		fscanf(asmFile, "%s", fd);//ingest the rest of the line of code
		fscanf(asmFile, "%s", fs);
		fscanf(asmFile, "%s", ft);

		Remove_Char(fd, ',');
		Remove_Char(fs, ',');

		fd = Get_FP_Reg_Num(fd);
		fs = Get_FP_Reg_Num(fs);
		ft = Get_FP_Reg_Num(ft);
	}
	else if (index == 2 || index == 3 || //fs, ft "c.eq.s fs, ft"
		index == 4 || index == 5 ||
		index == 6 || index == 7)
	{
		fscanf(asmFile, "%s", fs);
		fscanf(asmFile, "%s", ft);

		Remove_Char(fs, ',');
		Remove_Char(ft, ',');

		fd = "00000";
		fs = Get_FP_Reg_Num(fs);
		ft = Get_FP_Reg_Num(ft);
	}
	else
	{
		printf("error in %s", __func__);
	}

	//form instruction string
	printf("individual: op:%s, fmt:%s, ft:%s, fs:%s, fd:%s, funct:%s\n", opcode, fmt, ft, fs, fd, funct);
	char binary_instr[32];
	strcpy(binary_instr, opcode);
	strcat(binary_instr, fmt);
	strcat(binary_instr, ft);
	strcat(binary_instr, fs);
	strcat(binary_instr, fd);
	strcat(binary_instr, funct);

	//write to file
	fprintf(objFile, "%s\n", binary_instr);
	//fprintf(objFile, "%s	%s\n", binary_instr, Instruction_Set_FR[index].instruction);
	//printf("wrote %s\n", binary_instr);
}

void Process_FI_Instr(int index)
{
	char opcode[] = "010001";	//0x11 FI
	char* fmt = Instruction_Set_FI[index].fmt;
	char* ft = Instruction_Set_FI[index].funct;
	char* imm = "0000000000000000"; //16 bits
	int tmp;
								//"bclt/bclf label" no registers
	fscanf(asmFile, "%s", imm);	//label here;

	tmp = Get_Label_Line_Number(imm);	//label -> number -> binary string
	//printf("FI label no: %d, current PC: %d\n", tmp, PC);
	tmp = tmp - (PC + 0x04);				//pc relative addressing, BranchAddr = LabelAddress - (PC + 4)
	//printf("FI label no: %d, current PC: %d\n", tmp, PC);
	imm = Dec_To_SignExt_Bin(tmp);
	imm += 12; //Dec_To_SignExt_Bin returns 28-bit value, increment imm ptr by 12 to get required 16-bit value
	
	//form instruction string
	printf("individual: op:%s, fmt:%s, ft:%s, imm:%s\n", opcode, fmt, ft, imm);
	char binary_instr[32];
	strcpy(binary_instr, opcode);
	strcat(binary_instr, fmt);
	strcat(binary_instr, ft);
	strcat(binary_instr, imm);

	//write to file
	fprintf(objFile, "%s\n", binary_instr);
	//fprintf(objFile, "%s	%s\n", binary_instr, Instruction_Set_FI[index].instruction);
	//printf("wrote %s\n", binary_instr);
}

void main()
{
	fpos_t file_position;
	char instr[100];
	//int index = 0;
	
	

	asmFile = fopen("MIPS_program.asm", "r");
	objFile = fopen("MIPS_Object_file.txt", "w");
	fgetpos(asmFile, &file_position);	//pointer to beginning of file
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//PASS ONE FORM SYMBOL TABLE
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	while(!feof(asmFile))
	{
		fgets(instr, sizeof(instr), asmFile);	//get entire line of code
		//printf("%s", instr);
		Put_Symbol_Table(instr, PC);
		PC += 0x04;						//increment each line of code by 32 bits
	}


	for (int i = 0; Symbol_Table[i].address > 0; i++)	//print symbol table
	{
		printf("[%d]: %s  %d\n", i, Symbol_Table[i].label, Symbol_Table[i].address);
	}

	fsetpos(asmFile, &file_position);	//move file pointer to beginning of file for pass two
	PC = 0x00000000;					//reset PC to match line #
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//PASS TWO FORM MACHINE CODE + OBJECT FILE
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	while(!feof(asmFile))
	{
		fscanf(asmFile, "%s", instr);//get first word from line (check for instr)
		
		//check if instr is a label, run fscanf again to go to next word if it is
		if (strchr(instr, ':') != 0)	//if word contains ':' it is a label
		{
			printf("\nfound label %s, skipping...\n", instr);
			fscanf(asmFile, "%s", instr);//get first word from line (check for instr)
			PC += 0x04;	//increment each line of code by 32 bits
		}
		
		printf("\nstring:%s, size: %u, line: %u\n", instr, strlen(instr), PC/4);
		
		//check for instruction type
		//check against R type
		for (int i = 0; i < (sizeof(Instruction_Set_R) / sizeof(Instruction_Set_R[0])); i++)
		{
			if (strncmp(instr, Instruction_Set_R[i].instruction, sizeof(instr)) == 0)
			{
				Process_R_Instr(i);
				break;
			}
		}

		//check against I type
		for (int i = 0; i < (sizeof(Instruction_Set_I) / sizeof(Instruction_Set_I[0])); i++)
		{
			if (strncmp(instr, Instruction_Set_I[i].instruction, sizeof(instr)) == 0)
			{
				Process_I_Instr(i);
				break;
			}
		}

		//check against J type
		for (int i = 0; i < (sizeof(Instruction_Set_J) / sizeof(Instruction_Set_J[0])); i++)
		{
			if (strncmp(instr, Instruction_Set_J[i].instruction, sizeof(instr)) == 0)
			{
				Process_J_Instr(i);
				break;
			}
		}

		//check against FR type
		for (int i = 0; i < (sizeof(Instruction_Set_FR) / sizeof(Instruction_Set_FR[0])); i++)
		{
			if (strncmp(instr, Instruction_Set_FR[i].instruction, sizeof(instr)) == 0)
			{
				Process_FR_Instr(i);
				break;
			}
		}

		//check against FI type
		for (int i = 0; i < (sizeof(Instruction_Set_FI) / sizeof(Instruction_Set_FI[0])); i++)
		{
			if (strncmp(instr, Instruction_Set_FI[i].instruction, sizeof(instr)) == 0)
			{
				Process_FI_Instr(i);
				break;
			}
		}
		PC += 0x04;	//increment each line of code by 32 bits
	}

	// close the file
	fclose(asmFile);
	fclose(objFile);
}