`timescale 1ns / 1ps

module SCPU(	input clk,			//
					input myclk,
					input reset,
					input MIO_ready,
									
					input [31:0]inst_in,
					input [31:0]Data_in,	
									
					output mem_w,
					output[31:0]PC_out,
					output[31:0]Addr_out,
					output[31:0]Data_out, 
					output CPU_MIO,
					input INT,
					
					input [4:0] look_reg_addr,
					output [31:0] look_reg_data
				);
	
	
	
	// control signals
	reg [3:0] ALU;
	reg [1:0] Rt_Imme;
	reg [1:0] Result;
	reg [1:0] NewPC;
	reg [1:0] WRegAddr;
	reg WReg;
	reg Rs_Sa;
	
	reg Mem_R; // no use
	reg Mem_W;
	
	
	
	// program counter
	reg [31:0] PC;
	assign PC_out = PC;
	
	initial PC <= 32'b0;

	
	
	// decoder
	wire [31:0] ins = inst_in;
	wire [5:0] OpCode = ins[31:26];
	wire [4:0] Rs = ins[25:21];
	wire [4:0] Rt = ins[20:16];
	wire [4:0] Rd = ins[15:11];
	wire [4:0] Sa = ins[10:6];
	wire [5:0] Func = ins[5:0];
	wire [15:0] imme = ins[15:0];
	wire [25:0] target = ins[25:0];
	
	
	
	// Data Path
	reg [4:0] _tmp_WRegAddr;
	always @ (*) begin
		case (WRegAddr)
			2'd1: _tmp_WRegAddr <= Rt;
			2'd2: _tmp_WRegAddr <= 5'd31;
			default: _tmp_WRegAddr <= Rd;
		endcase
	end
	
	wire [31:0] RegWriteData;
	wire [31:0] DataRs, DataRt;
	
	Regs RegFile(.clk(clk), .rst(reset), .L_S(WReg), .R_addr_A(Rs), .R_addr_B(Rt), 
		.Wt_addr(_tmp_WRegAddr), .Wt_data(RegWriteData), .rdata_A(DataRs), .rdata_B(DataRt),
		.look_addr(look_reg_addr), .look_data(look_reg_data));
	
	
	reg [31:0] _tmp_ALU_B;
	always @ (*) begin
		case (Rt_Imme)
		//	2'd0: _tmp_ALU_B <= DataRt;
		//	2'd1: _tmp_ALU_B <= {27'b0, Sa};
			2'd2: _tmp_ALU_B <= {16'b0, imme};
			2'd3: _tmp_ALU_B <= {{16{imme[15]}}, imme};
			default: _tmp_ALU_B <= DataRt;
		endcase
	end
	
	wire [31:0] _tmp_ALU_A = (Rs_Sa == 1'b0) ? DataRs : {27'b0, Sa};
	
	wire [31:0] ALU_R;
	wire ALU_Z;
	
	MyALU myALU(.ALU_Operation(ALU), .A(_tmp_ALU_A), .B(_tmp_ALU_B), .res(ALU_R), .zero(ALU_Z));
	
	
	assign Addr_out = ALU_R;
	assign Data_out = DataRt;
	assign mem_w = Mem_W;
	wire [31:0] Mem_Data = Data_in;
	
	
	reg [31:0] _tmp_WRegData;
	always @ (*) begin
		case (Result)
			3'd0: _tmp_WRegData <= ALU_R;
			3'd1: _tmp_WRegData <= Mem_Data;
			3'd2: _tmp_WRegData <= {imme, 16'b0};
			3'd3: _tmp_WRegData <= PC + 32'd4;
		endcase
	end
	
	assign RegWriteData = _tmp_WRegData;
	
	wire [31:0] PC_Plus4 = PC + 32'd4;
	always @ (posedge myclk) begin
		case (NewPC)
			2'd0: PC <= PC_Plus4;
			2'd1: PC <= DataRs;
			2'd2: PC <= PC_Plus4 + {{14{imme[15]}}, imme, 2'b0};
			2'd3: PC <= {PC_Plus4[31:28], target, 2'b0};
		endcase
	end
	
	
	
	
	// controller
	parameter OPC_SW = 6'h2B;
	parameter OPC_BEQ = 6'h4;
	parameter OPC_BNE = 6'h5;
	parameter OPC_J = 6'h2;
	parameter FUNC_JR = 6'h8;
	
	always @ (*) begin
		if ((OpCode == 6'd0 && Func == FUNC_JR) || OpCode == OPC_SW || OpCode == OPC_BEQ ||
			  OpCode == OPC_BNE || OpCode == OPC_J) WReg <= 1'b0;
		else WReg <= 1'b1;
	end
	

	parameter FUNC_SLL = 6'h0;
	parameter FUNC_SRL = 6'h2;
	parameter FUNC_SRA = 6'h3;
	
	parameter OPC_ADDIU = 6'h9;
	parameter OPC_ANDI = 6'hC;
	parameter OPC_ORI = 6'hD;
	parameter OPC_XORI = 6'hE;
	parameter OPC_SLTIU = 6'hB;
	
	parameter OPC_ADDI = 6'h8;
	parameter OPC_SLTI = 6'hA;
	parameter OPC_LW = 6'h23;
	parameter OPC_LH = 6'h21;
	parameter OPC_LHU = 6'h25;
	parameter OPC_LB = 6'h20;
	parameter OPC_LBU = 6'h24;
	
	always @ (*) begin
/*		if (OpCode == 6'd0 && (Func == FUNC_SLL || Func == FUNC_SRL || Func == FUNC_SRA)) Rt_Imme <= 2'd1;
		else */if (OpCode == OPC_ADDIU || OpCode == OPC_ANDI || OpCode == OPC_ORI ||
					OpCode == OPC_XORI || OpCode == OPC_SLTIU) Rt_Imme <= 2'd2;
		else if (OpCode == OPC_ADDI || OpCode == OPC_SLTI || OpCode == OPC_LW || OpCode == OPC_LH ||
					OpCode == OPC_LHU || OpCode == OPC_LB || OpCode == OPC_LBU || OpCode == OPC_SW) Rt_Imme <= 2'd3;
		else Rt_Imme <= 2'd0;
	end
	
	always @ (*) begin
		if (OpCode == 6'd0 && (Func == FUNC_SLL || Func == FUNC_SRL || Func == FUNC_SRA)) Rs_Sa <= 2'd1;
		else Rs_Sa <= 2'd0;
	end
	
	
	parameter FUNC_JALR = 6'h9;
	parameter OPC_JAL = 6'h3;
	
	always @ (*) begin
		if (OpCode == 6'd0 && Func != FUNC_JR && Func != FUNC_JALR) WRegAddr <= 2'd0;
		else if ((OpCode == 6'd0 && Func == FUNC_JALR) || OpCode == OPC_JAL) WRegAddr <= 2'd2;
		else WRegAddr <= 2'd1;
	end
	
	always @ (*) begin
		if (OpCode == OPC_LW || OpCode == OPC_LH || OpCode == OPC_LHU || 
		    OpCode == OPC_LB || OpCode == OPC_LBU) Mem_R <= 1'b1;
		else Mem_R <= 1'b0;
		if (OpCode == OPC_SW) Mem_W <= 1'b1;
		else Mem_W <= 1'b0;
	end
	
	
	parameter OPC_LUI = 6'hF;
	
	always @ (*) begin
		if (OpCode == 6'd0 && Func == FUNC_JALR) Result <= 3'd3;
		else begin
			case (OpCode)
				OPC_LW: Result <= 3'd1;
				OPC_LUI: Result <= 3'd2;
				OPC_JAL: Result <= 3'd3;
				default: Result <= 3'd0;
			endcase
		end
	end
	
	
	always @ (*) begin
		if (OpCode == 6'd0 && (Func == FUNC_JR || Func == FUNC_JALR)) NewPC <= 2'd1;
		else if (OpCode == OPC_J || OpCode == OPC_JAL) NewPC <= 2'd3;
		else if (OpCode == OPC_BEQ && ALU_Z == 1'b1 || OpCode == OPC_BNE && ALU_Z == 1'b0) NewPC <= 2'd2;
		else NewPC <= 2'd0;
	end
	
	
	parameter FUNC_SUB = 6'h22;
	parameter FUNC_SUBU = 6'h23;
	parameter FUNC_SLT = 6'h2A;
	parameter FUNC_SLTU = 6'h2B;
	parameter FUNC_AND = 6'h24;
	parameter FUNC_OR = 6'h25;
	parameter FUNC_XOR = 6'h26;
	parameter FUNC_NOR = 6'h27;
	parameter FUNC_MULT = 6'h18;
	parameter FUNC_MULTU = 6'h19;
	parameter FUNC_DIV = 6'h1A;
	parameter FUNC_DIVU = 6'h1B;
	parameter FUNC_SLLV = 6'h4;
	parameter FUNC_SRLV = 6'h6;
	parameter FUNC_SRAV = 6'h7;
	
	parameter ALU_AND = 4'd0;
	parameter ALU_OR = 4'd1;
	parameter ALU_ADD = 4'd2;
	parameter ALU_XOR = 4'd3;
	parameter ALU_NOR = 4'd4;
	parameter ALU_SRL = 4'd5;
	parameter ALU_SUB = 4'd6;
	parameter ALU_SLT = 4'd7;
	parameter ALU_MULT = 4'd8;
	parameter ALU_DIV = 4'd9;
	parameter ALU_DIVU = 4'd10;
	parameter ALU_A = 4'd11;
	parameter ALU_B = 4'd12;
	parameter ALU_SRA = 4'd13;
	parameter ALU_SLL = 4'd14;
	parameter ALU_SLTU = 4'd15;
	
	always @ (*) begin
		if (OpCode == 6'b0) begin
			case (Func)
				FUNC_SUB: ALU <= ALU_SUB;
				FUNC_SUBU: ALU <= ALU_SUB;
				FUNC_AND: ALU <= ALU_AND;
				FUNC_OR: ALU <= ALU_OR;
				FUNC_NOR: ALU <= ALU_NOR;
				FUNC_XOR: ALU <= ALU_XOR;
				FUNC_SLT: ALU <= ALU_SLT;
				FUNC_SLTU: ALU <= ALU_SLTU;
				FUNC_SLL: ALU <= ALU_SLL;
				FUNC_SRL: ALU <= ALU_SRL;
				FUNC_SRA: ALU <= ALU_SRA;
				FUNC_SLLV: ALU <= ALU_SLL;
				FUNC_SRLV: ALU <= ALU_SRL;
				FUNC_SRAV: ALU <= ALU_SRA;
				FUNC_MULT: ALU <= ALU_MULT;
				FUNC_MULTU: ALU <= ALU_MULT;
				FUNC_DIV: ALU <= ALU_DIV;
				FUNC_DIVU: ALU <= ALU_DIVU;
				default: ALU <= ALU_ADD;
			endcase
		end
		else if (OpCode == OPC_BEQ || OpCode == OPC_BNE) ALU <= ALU_SUB;
		else begin
			case (OpCode)
				OPC_ANDI: ALU <= ALU_AND;
				OPC_ORI: ALU <= ALU_OR;
				OPC_XORI: ALU <= ALU_XOR;
				OPC_SLTI: ALU <= ALU_SLT;
				OPC_SLTIU: ALU <= ALU_SLTU;
				default: ALU <= ALU_ADD;
			endcase
		end
	end
	
endmodule














