#include<iostream>
#include<string.h>
#include<vector>
#include<bitset>
#include<fstream>


using namespace std;

#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize 65536


class RF
{
  public:
    bitset<32> ReadData1, ReadData2; 
    RF()
    { 
      Registers.resize(32);  
      Registers[0] = bitset<32> (0);  
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {   
      // TODO: implement!       
		if (WrtEnable == 1) {
			Registers[WrtReg.to_ulong()] = WrtData; // write data into register
		}

		ReadData1 = Registers[RdReg1.to_ulong()];  // read register. RdReg1.to_ulong() transfer bitset to decimal
		ReadData2 = Registers[RdReg2.to_ulong()];

    }
	
    void OutputRF()
    {
      ofstream rfout;
      rfout.open("RFresult.txt",std::ios_base::app);
      if (rfout.is_open())
      {
        rfout<<"A state of RF:"<<endl;
        for (int j = 0; j<32; j++)
        {        
          rfout << Registers[j] << endl;
        }

      }
      else cout<<"Unable to open file";
      rfout.close();

    }     
  private:
    vector<bitset<32> >Registers;
};


class ALU
{
  public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {   
      // TODO: implement!
		//Regular expression: ALUOP={(001,addu);(,subu).....}
		//ALUresult= oprand1 oprand2

		unsigned long aluResult;

		if (ALUOP == 001) //ADDU = 1
		{
			aluResult = oprand1.to_ulong() + oprand2.to_ulong();  // + do not apply to bitset
		}
		else if(ALUOP == SUBU) //SUBU 3
		{
			aluResult = oprand1.to_ulong() - oprand2.to_ulong();
		}
		else if (ALUOP == AND) { //AND 4
			aluResult = oprand1.to_ulong() & oprand2.to_ulong();
		}
		else if (ALUOP == OR) { //OR 5
			aluResult = oprand1.to_ulong() | oprand2.to_ulong();
		}
		else if (ALUOP == NOR) { //NOR 7
			aluResult = ~(oprand1.to_ulong() | oprand2.to_ulong());
		}

		bitset<32> alures((int)aluResult);  // transfer long to bitset
        ALUresult = alures;

      return ALUresult;
    }            
};


class INSMem
{
  public:
    bitset<32> Instruction;
    INSMem()
    {      
      IMem.resize(MemSize); 
      ifstream imem;
      string line;
      int i=0;
      imem.open("imem.txt");
      if (imem.is_open())
      {
        while (getline(imem,line))
        {      
          IMem[i] = bitset<8>(line);
          i++;
        }

      }
      else cout<<"Unable to open file";
      imem.close();

    }

    bitset<32> ReadMemory (bitset<32> ReadAddress) 
    {    
      // TODO: implement!
      // (Read the byte at the ReadAddress and the following three byte).
		int address = (int)ReadAddress.to_ulong();
		string readString;
		for (int i = 0; i < 4; i++) {
			readString += IMem[address].to_string();
			address++;
		}
		bitset<32> instr(readString);
		Instruction = instr;

      return Instruction;     
    }     

  private:
    vector<bitset<8> > IMem;

};

class DataMem    
{
  public:
    bitset<32> readdata;  
    DataMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("dmem.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {      
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open file";
      dmem.close();

    }  
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem) 
    {                                       // In memory, 8 bits continuously follow 8 bits, address is also continuous.
      // TODO: implement!	
		int address = (int)Address.to_ulong();
		
		if (readmem.to_ulong() == 1) {  //read data memory
			string readString;
			for (int i = 0; i < 4; i++) {  //read this address and the following 3 address
				readString += DMem[address].to_string();
				address++;
			}
			bitset<32> data(readString);
			readdata = data;		
		}
		else if (writemem == 1) {  //write data memory
		   //split the write data in to 4 bytes, and store them continuously into 4 address in memory
			string writeString;
			for (int i = 0; i < 32; i = i + 8) {
			    writeString = WriteData.to_string().substr(i, 8);
				bitset<8> writeData(writeString);
				DMem[address] = writeData;
				address++;
			}
		}
		
      return readdata;     
    }   

    void OutputDataMem()
    {
      ofstream dmemout;
      dmemout.open("dmemresult.txt");
      if (dmemout.is_open())
      {
        for (int j = 0; j< 1000; j++)
        {     
          dmemout << DMem[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      dmemout.close();

    }             

  private:
    vector<bitset<8> > DMem;

};  



int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;

  
  bitset<32> pc = bitset<32>(0);  //defines a 32 bit pc that is initialized to zero, pc pointed to a 32 bits address with stores 8 bits instruction
  bitset<32> instruction; //instruction
  string opcode;
  string imm;
  bitset<32> $rs, $rd, $rt, aluResult;
  bitset<3> addop(ADDU), subuop(SUBU), andop(AND), orop(OR), norop(NOR);


  while (1)
  {
	  
    // Fetch
	  instruction = myInsMem.ReadMemory(pc);
    // If current insturciton is "11111111111111111111111111111111", then break;
	  if (instruction.to_string() == "11111111111111111111111111111111") {
		  pc = bitset<32> (pc.to_ulong() + 4);  // getch next instruction and judge if is NULL
		  instruction = myInsMem.ReadMemory(pc); 
		  if(instruction == NULL) break;
	  }
	  

	  //opcode = inst[31:26];
	  opcode= instruction.to_string().substr(0, 6); //substring start from position 0, read 6 chracter
	  
	  if (opcode == "000000") { // R Type. In C/C++, 0x means Hex   opcode == 0x00
		 // decode(Read RF)
		  bitset<3> funct(instruction.to_string().substr(29,3)); //this lab only needs the last three bits of funct 
		  bitset<5> rs(instruction.to_string().substr(6, 5));
		  bitset<5> rt(instruction.to_string().substr(11, 5));
		  bitset<5> rd(instruction.to_string().substr(16, 5));

		  myRF.ReadWrite(rs, rt, NULL, NULL, 0);
		  $rs = myRF.ReadData1;    //R[rs]
		  $rt = myRF.ReadData2;    //R[rt]

		  if (funct == 1) { //addu: R[rd] ← R[rs] + R[rt]
			  // Execute
			  aluResult = myALU.ALUOperation(addop, $rs, $rt); // R[rs] + R[rt]
		  }		 
		  else if (funct == 3) { //subu
			  aluResult = myALU.ALUOperation(subuop, $rs, $rt); // R[rs] - R[rt]		  
		  }
		  else if (funct == 4) { //and
			  aluResult = myALU.ALUOperation(andop, $rs, $rt); // R[rs] & R[rt]		  
		  }
		  else if (funct == 5) { //or
			  aluResult = myALU.ALUOperation(orop, $rs, $rt); // R[rs] | R[rt]		  
		  }
		  else if (funct == 7) { //nor
			  aluResult = myALU.ALUOperation(norop, $rs, $rt); // ~(R[rs] | R[rt])		  
		  }

		  // Write back to RF
		  myRF.ReadWrite(NULL, NULL, rd, aluResult, 1); //R[rd] ← R[rs] ALUOP R[rt]

		  pc = bitset<32>(pc.to_ulong() + 4); //pc = pc + 4, pc.to_ulong() transfer bitset to decimal
	  
	  }
	  else if (opcode == "000010")  { //J type
		  // j address: PC ← JumpAddress   JumpAddress = { (PC+4)[31:28], address, 00 }
		  
		  pc = bitset<32> (pc.to_ulong() + 4); //pc + 4
		  string pcString = pc.to_string().substr(0, 4);  //(PC+4)[31:28]
		  string jumpAddressStr = pcString + instruction.to_string().substr(6, 26) + "00"; //JumpAddress = { (PC+4)[31:28], address, 2’b0 }
		  bitset<32> jumpAddress(jumpAddressStr);
		  pc = jumpAddress;  //PC ← JumpAddress 

	  }
	  else { // I type
		  bitset<5> rs(instruction.to_string().substr(6, 5)); //rs
		  bitset<5> rt(instruction.to_string().substr(11, 5)); //rt
		  //R[rs]
		  myRF.ReadWrite(rs, NULL, NULL, NULL, 0);
		  $rs = myRF.ReadData1;

		  //{SignExtend, imm}
		  imm = instruction.to_string().substr(16, 16); // 16 bits imm
		  string finalImmStr; //32 bits imm
		  if (imm.at(0) == '0') {
			  finalImmStr = "0000000000000000" + imm;
		  }
		  else if (imm.at(0) == '1') {
			  finalImmStr = "1111111111111111" + imm;
		  }
		  bitset<32> finalImm(finalImmStr);

		  aluResult = myALU.ALUOperation(addop, $rs, finalImm); //R[rs] + {SignExtend, imm}
		  
		  if (opcode == "100011") { //lw: R[rt] ← Mem[ {SignExtend, imm} + R[rs] ]
			  
			  bitset<32> operand1 = myDataMem.MemoryAccess(aluResult, NULL, 1, 0); //Mem[R[rs] + {SignExtend, imm}]

			  myRF.ReadWrite(NULL, NULL, rt, operand1, 1); //Register[rt] <- Mem[R[rs] + {SignExtend, imm}]
			  
			  pc = bitset<32> (pc.to_ulong() + 4); //pc = pc + 4, pc.to_ulong() transfer bitset to decimal
		  
		  }
		  else if (opcode == "101011") { //sw: MEM[R[rs] + {SignExtend, imm}] <- R[rt]
			  //R[t]
			  myRF.ReadWrite(NULL, rt, NULL, NULL, 0);
			  $rt = myRF.ReadData2;

			  bitset<32> returnn = myDataMem.MemoryAccess(aluResult, $rt, 0, 1); //MEM[R[rs] + {SignExtend, imm}] <- R[rt]
			  pc = bitset<32> (pc.to_ulong() + 4); //pc = pc + 4
		  
		  }
		  else if (opcode == "001001") {//addiu:R[rt] ← R[rs] + {SignExtend, imm};
			  myRF.ReadWrite(NULL, NULL, rt, aluResult, 1);
			  pc = bitset<32> (pc.to_ulong() + 4); //pc = pc + 4
		  }
		  else if (opcode == "000100") { //beq:  // if (R[rs]==R[rt]) PC ← PC + 4 + {SignExtendImm, 00} 
												// else PC ← PC + 4
			  myRF.ReadWrite(NULL, rt, NULL, NULL, 0);
			  $rt = myRF.ReadData2;

			  if ($rs.to_ulong() == $rt.to_ulong()) {
				bitset<32> finalImm(finalImmStr.substr(2, 30) + "00");  //{SignExtendImm, 00} means lest shift SignExtendImm 2 bits, which means get the last 30 bits then add 00 in the end
				pc = bitset<32> (pc.to_ulong() + 4 + finalImm.to_ulong()); //pc = pc + 4 + {SignExtendImm, 00}			  
			  }
			  else {
				  pc = bitset<32> (pc.to_ulong() + 4); //pc = pc + 4			  
			  }

		  }
	  
  }
    
    myRF.OutputRF(); // dump RF;    
}

  myDataMem.OutputDataMem(); // dump data mem

  return 0;
}
