`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/05/28 15:11:21
// Design Name: 
// Module Name: SCPU
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module SCPU(
    input         clk,
    input         rst,
    input  [31:0] inst,     //指令
    input  [63:0] data_in,  //datamemory输出数据
    output [63:0] addr_out, //需要访问的数据内存地址
    output [63:0] data_out, //需要存入数据内存的数据
    output [63:0] pc_out,   //下一条指令的地址
    output        mem_write,//信号：是否需要数据内存写入
    output [1:0]  LBinRam,  //信号：写入Ram的数据长度 00:64 10:32 01:8 （bits）
    output [63:0] ra,       //32个32位寄存器
    output [63:0] sp,
    output [63:0] gp,
    output [63:0] tp,
    output [63:0] t0,
    output [63:0] t1,
    output [63:0] t2,
    output [63:0] s0,
    output [63:0] s1,
    output [63:0] a0,
    output [63:0] a1,
    output [63:0] a2,
    output [63:0] a3,
    output [63:0] a4,
    output [63:0] a5,
    output [63:0] a6,
    output [63:0] a7,
    output [63:0] s2,
    output [63:0] s3,
    output [63:0] s4,
    output [63:0] s5,
    output [63:0] s6,
    output [63:0] s7,
    output [63:0] s8,
    output [63:0] s9,
    output [63:0] s10,
    output [63:0] s11,
    output [63:0] t3,
    output [63:0] t4,
    output [63:0] t5,
    output [63:0] t6,
    output [63:0] WB_data          
    );
   
    wire    or_2_res;                   //判断如果是branch和jal指令，是否满足条件
    wire    [63:0]  PC_out_0;           //pc+4
    wire    [63:0]  PC_out_1;           //pc+Imm_out
    wire    [63:0]  PC_out_2;           //表示branch经过选择后的pc

    wire [3:0] ID_alu_op, EX_alu_op;                                         
    wire [1:0] ID_pc_src, EX_pc_src, MEM_pc_src;
    wire [1:0] ID_mem_to_reg, EX_mem_to_reg, MEM_mem_to_reg, WB_mem_to_reg;                                       
    wire ID_reg_write, EX_reg_write, MEM_reg_write, WB_reg_write;
    wire ID_alu_src, EX_alu_src;
    wire ID_branch, EX_branch, MEM_branch;
    wire ID_b_type, EX_b_type, MEM_b_type;
    wire ID_b_type3;       
    wire [1:0] ID_b_type2, EX_b_type2, MEM_b_type2;
    wire [1:0] ID_LBinReg, EX_LBinReg, MEM_LBinReg, WB_LBinReg;
    wire [1:0] ID_LBinRam, EX_LBinRam;                                     
    wire ID_mem_read, EX_mem_read, MEM_mem_read, WB_mem_read;                                              
    wire ID_mem_write, EX_mem_write, MEM_mem_write;           
                                      
    
    

    wire [63:0] IF_PC, ID_PC, EX_PC, MEM_PC, WB_PC;                    //PC
    wire [31:0] IF_IR, ID_IR, EX_IR, MEM_IR, WB_IR;                    //指令
    wire [63:0] ID_A, ID_B, EX_A, EX_B,MEM_A, MEM_B;                  //寄存器存储值
    wire [63:0] ID_A_temp, ID_B_temp;
    wire [63:0] ID_IMM, EX_IMM, MEM_IMM, WB_IMM;                                 //扩展后的立即数
    wire [63:0] ID_ADDER, EX_ADDER, EX_ALUout, MEM_ALUout, WB_ALUout;//ALU和ADDER的输出结果
    wire EX_ALUzero, MEM_ALUzero;                               //信号：ALU结果是否为0
    wire [63:0] MEM_readdata, WB_readdata;                      //从数据内存中读取的数据
    wire [63:0] ID_CSR_read_data , EX_CSR_read_data , MEM_CSR_read_data, WB_CSR_read_data ;
    wire [63:0] MEM_CSR_read_data_temp;


    wire [63:0] read_data_3;                                    //经过ALUB接口选择后的数据
    wire [63:0] PCadd4, WB_PCadd4;
   // wire [31:0] WB_data;
    wire [63:0] WB_data_temp;
    wire [63:0] final_PC_out_temp, final_PC_out;
    wire [63:0] pc_update;
    wire [63:0] new_pc;
    wire [63:0] PC_to_reg;
    wire [63:0] PC_jalr;
    wire [63:0] CSR_pcsrc ;
   

    wire [4:0] ID_rs1, ID_rs2, ID_rd;
    wire [4:0] EX_rs1, EX_rs2, EX_rd;
    wire [4:0] MEM_rd;
    wire [4:0] WB_rd;
    wire [63:0] final_A, final_B;
    wire [1:0] forward_a, forward_b;
    wire [1:0] CSR_forward_a, CSR_forward_b;
    wire control_write;
    wire PC_write;
    wire jmp_or_not;
    wire EX_jmp_or_not;
    wire ill_or_not;
    wire ill_instr_temp, ill_instr;
    wire [1:0] ID_pc_src_temp;
    wire ID_reg_write_temp;
    wire [4:0] ID_alu_op_temp;
    wire ID_alu_src_temp;
    wire [1:0] ID_mem_to_reg_temp;
    wire ID_mem_read_temp;
    wire ID_mem_write_temp;
    wire ID_branch_temp;
    wire ID_b_type_temp;
    wire ID_b_type3_temp;
    wire [1:0]  ID_b_type2_temp;
    wire [1:0] ID_LBinReg_temp, ID_LBinRam_temp;
    wire [31:0] IF_IR_temp;
    wire [31:0] ID_IR_temp;



    wire IF_predict_jmp, ID_predict_jmp ;
    wire IF_if_foundBTB , ID_if_foundBTB;
    wire [63:0] IF_predict_jmp_PC, ID_predict_jmp_PC;

    
    
    //IF
                                          //获取PC

    assign IF_IR_temp = inst;
    assign  or_2_res = jmp_or_not | (~ID_branch & (ID_pc_src == 2'b10)) ;
    
    MUX_pc mux2
    (
        .T0(PCadd4),
        .T1(ID_ADDER),
        .T3(ID_PC+4),
        .s(or_2_res),
        .ID_IR(ID_IR),
        .ID_jmp_or_not(jmp_or_not),
        .ID_predict_jmp(ID_predict_jmp),
        .o(PC_out_2)
    );
    MUX4T1      mux4_1
    (
        .T0(PCadd4),          //00表示来自pc+4
        .T1(PC_jalr),          //01表示来自JALR
        .T2(PC_out_2),          //10表示来自JAL（含branch）
        .T3(64'b0),          
        .Sel(ID_pc_src),
        .o(final_PC_out_temp)
    );

    MUX_jmp MUX_jmp
    (
        .PC_temp(final_PC_out_temp),
        .PC_final(final_PC_out),
        .predict_jmp(IF_predict_jmp),
        .predict_jmp_PC(IF_predict_jmp_PC)
    );

    PC_reg      PcReg
    (
        .clk(clk),
        .CE(1'b1),
        .PC_write(PC_write),
        .CSR_pcsrc(CSR_pcsrc),
        .ID_IR(ID_IR),
        .rst(rst),
        .old(final_PC_out),
        .stall(pc_out),
        .new(pc_out),
        .ill_or_not(ill_or_not)
    );



    Add_32  add_32_T3                    //执行pc+4
    (           
        .former(pc_out),
        .addend(64'd4),
        .latter(PCadd4)
    );
    
    assign IF_PC = pc_out;

    
    BHTtable bht1 
    (
        .rst(rst),
        .clk(clk),
        .PC(IF_PC),
        .IF_IR(IF_IR),
        .add_PC(ID_PC),
        .jmp_or_not(jmp_or_not),
        .if_foundBTB(IF_if_foundBTB),
        .we(ID_branch),
        .predict_jmp(IF_predict_jmp)
    );

    

    BTBtable btb1 
    (
        .rst(rst),
        .clk(clk),
        .PC(IF_PC),
        .add_ID_PC(ID_PC),
        .add_EX_PC(EX_PC),
        .ID_IR(ID_IR),
        .EX_IR(EX_IR),
        .ID_jmp_or_not(jmp_or_not),
        .EX_jmp_or_not(EX_jmp_or_not),
        .ID_we(ID_branch),
        .EX_we(EX_branch),
        .add_ID_jmp_PC(ID_ADDER),
        .add_EX_jmp_PC(EX_ADDER),
        .if_foundBTB(IF_if_foundBTB),
        .jmp_PC(IF_predict_jmp_PC)
    );

   
    
    REG_IFID ifid(clk, rst, IF_PC, IF_IR, IF_predict_jmp, IF_if_foundBTB, IF_predict_jmp_PC, ID_PC, ID_IR_temp, ID_predict_jmp, ID_if_foundBTB, ID_predict_jmp_PC);                   //段间寄存器IF/ID
    
    //ID

    Detection detect1 (
        .ID_rs1(ID_rs1),
        .ID_rs2(ID_rs2),
        .EX_rd(EX_rd),
        .ID_rd(ID_rd),
        .ID_mem_read(ID_mem_read),
        .ID_predict_jmp(ID_predict_jmp),
        .ID_if_foundBTB(ID_if_foundBTB),
        .IF_IR_in(IF_IR_temp),
        .IF_IR_out(IF_IR),
        .ID_mem_to_reg(ID_mem_to_reg),
        .control_write(control_write),
        .PC_write(PC_write),
        .jmp_or_not(jmp_or_not),
        .ID_branch(ID_branch),
        .EX_mem_read(EX_mem_read),
        .ID_IR_in(ID_IR_temp),
        .ID_IR_out(ID_IR),
        .ill_or_not(ill_or_not)
    );

    Control control (                                            //控制信号译码
            .op_code(ID_IR[6:0]),
            .funct3(ID_IR[14:12]),
            .funct7_5(ID_IR[30]),
            .pc_src(ID_pc_src_temp),         // 2'b00 表示pc的数据来自pc+4, 2'b01 表示数据来自JALR跳转地址, 2'b10表示数据来自JAL跳转地址(包括branch). branch 跳转根据条件决定
            .reg_write(ID_reg_write_temp),   // 1'b1 表示写寄存器
            .alu_src_b(ID_alu_src_temp),   // 1'b1 表示ALU B口的数据源来自imm, 1'b0表示数据来自Reg[rs2]
            .alu_op(ID_alu_op_temp),         // 用来控制ALU操作，具体请看AluOp.vh中对各个操作的编码
            .mem_to_reg(ID_mem_to_reg_temp), // 2'b00 表示写回rd的数据来自ALU, 2'b01表示数据来自imm, 2'b10表示数据来自pc+4, 2'b11 表示数据来自data memory
            .mem_write(ID_mem_write_temp),   // 1'b1 表示写data memory, 1'b0表示读data memory
            .branch(ID_branch_temp),         // 1'b1 表示是branch类型的指令
            .b_type(ID_b_type_temp),          // 1'b1 表示beq, 1'b0 表示bne
            .b_type2(ID_b_type2_temp),             // 2'b01表示bge, 2'b10表示bgeu, 2'b11表示blt和bltu
            .b_type3(ID_b_type3_temp),             // 1'b0表示blt, 1'b1表示bltu
            .mem_read(ID_mem_read_temp),
            .ill_instr(ill_instr_temp),
            .LBinReg(ID_LBinReg_temp),
            .LBinRam(ID_LBinRam_temp)
            );

    MUX2T1_control mux_control (
        .control_write(control_write),
        .ID_pc_src(ID_pc_src),
        .ID_reg_write(ID_reg_write),
        .ID_alu_src(ID_alu_src),
        .ID_alu_op(ID_alu_op),
        .ID_mem_to_reg(ID_mem_to_reg),
        .ID_mem_write(ID_mem_write),
        .ID_mem_read(ID_mem_read),
        .ID_branch(ID_branch),
        .ID_b_type(ID_b_type),
        .ID_b_type2(ID_b_type2),
        .ID_b_type3(ID_b_type3),
        .ID_LBinReg_temp(ID_LBinReg_temp),
        .ID_LBinRam_temp(ID_LBinRam_temp),
        .ID_LBinReg(ID_LBinReg),
        .ID_LBinRam(ID_LBinRam),
        .ID_pc_src_temp(ID_pc_src_temp),
        .ID_reg_write_temp(ID_reg_write_temp),
        .ID_alu_src_temp(ID_alu_src_temp),
        .ID_alu_op_temp(ID_alu_op_temp),
        .ID_mem_to_reg_temp(ID_mem_to_reg_temp),
        .ID_mem_write_temp(ID_mem_write_temp),
        .ID_mem_read_temp(ID_mem_read_temp),
        .ID_branch_temp(ID_branch_temp),
        .ID_b_type_temp(ID_b_type_temp),
        .ID_b_type2_temp(ID_b_type2_temp),
        .ID_b_type3_temp(ID_b_type3_temp),
        .ill_instr(ill_instr),
        .ill_instr_temp(ill_instr_temp)
    );

    


    CSRregs csr (
        .clk(clk),
        .rst(rst),
        .ill_or_not(ill_or_not),
        .ID_IR(ID_IR),
        .now_pc(pc_out),
        .CSR_write_data(ID_A),
        .CSR_read_data(ID_CSR_read_data),
        .change_pc(CSR_pcsrc)
    );

    ill_detection ill (
        .ID_IR(ID_IR),
        .ill_instr(ill_instr),
        .ill_or_not(ill_or_not)
    );



    assign ID_rs1 = ID_IR[19:15];
    assign ID_rs2 = ID_IR[24:20];
    assign ID_rd = ID_IR[11:7];
    
    Regs      reg1                                             //寄存器读取
    (   
        .clk(clk),
        .rst(rst),
        .LBinReg(WB_LBinReg),
        .read_addr_1(ID_IR[19:15]),
        .read_addr_2(ID_IR[24:20]),
        .read_data_1(ID_A_temp),
        .read_data_2(ID_B_temp),
        .we(WB_reg_write),
        .write_addr(WB_IR[11:7]),
        .write_data(WB_data),
        .ra(ra),              // output wire [31 : 0] ra
        .sp(sp),              // output wire [31 : 0] sp
        .gp(gp),              // output wire [31 : 0] gp
        .tp(tp),              // output wire [31 : 0] tp
        .t0(t0),              // output wire [31 : 0] t0
        .t1(t1),              // output wire [31 : 0] t1
        .t2(t2),              // output wire [31 : 0] t2
        .s0(s0),              // output wire [31 : 0] s0
        .s1(s1),              // output wire [31 : 0] s1
        .a0(a0),              // output wire [31 : 0] a0
        .a1(a1),              // output wire [31 : 0] a1
        .a2(a2),              // output wire [31 : 0] a2
        .a3(a3),              // output wire [31 : 0] a3
        .a4(a4),              // output wire [31 : 0] a4
        .a5(a5),              // output wire [31 : 0] a5
        .a6(a6),              // output wire [31 : 0] a6
        .a7(a7),              // output wire [31 : 0] a7
        .s2(s2),              // output wire [31 : 0] s2
        .s3(s3),              // output wire [31 : 0] s3
        .s4(s4),              // output wire [31 : 0] s4
        .s5(s5),              // output wire [31 : 0] s5
        .s6(s6),              // output wire [31 : 0] s6
        .s7(s7),              // output wire [31 : 0] s7
        .s8(s8),              // output wire [31 : 0] s8
        .s9(s9),              // output wire [31 : 0] s9
        .s10(s10),            // output wire [31 : 0] s10
        .s11(s11),            // output wire [31 : 0] s11
        .t3(t3),              // output wire [31 : 0] t3
        .t4(t4),              // output wire [31 : 0] t4
        .t5(t5),              // output wire [31 : 0] t5
        .t6(t6)              // output wire [31 : 0] t6
    );
    
    ImmGen  imm_fun                                              //立即数生成器
    (
        .instr(ID_IR),
        .pc(ID_PC),
        .Imm_out(ID_IMM)
    );

    Forward_for_B forwardingB (
        .EX_IR(EX_IR),
        .ID_IR(ID_IR),
        .MEM_IR(MEM_IR),
        .WB_IR(WB_IR),
        .EX_CSR_read_data(EX_CSR_read_data),
        .MEM_CSR_read_data(MEM_CSR_read_data),
        .ID_branch(ID_branch),
        .MEM_mem_read(MEM_mem_read),
        .WB_mem_read(WB_mem_read),
        .ID_rs1(ID_rs1),
        .ID_rs2(ID_rs2),
        .MEM_rd(MEM_rd),
        .WB_data(WB_data),
        .WB_rd(WB_rd),
        .WB_readdata(WB_readdata),
        .ID_A(ID_A),
        .ID_A_temp(ID_A_temp),
        .ID_B(ID_B),
        .ID_B_temp(ID_B_temp),
        .EX_reg_write(EX_reg_write),
        .EX_mem_read(EX_mem_read),
        .EX_rd(EX_rd),
        .MEM_readdata(MEM_readdata),
        .EX_ALUout(EX_ALUout),
        .MEM_reg_write(MEM_reg_write),
        .WB_reg_write(WB_reg_write),
        .MEM_ALUout(MEM_ALUout)
    );

    Jmp_detection jmp ( 
        .ID_branch(ID_branch),
        .ID_b_type(ID_b_type),
        .ID_b_type2(ID_b_type2),
        .ID_b_type3(ID_b_type3),
        .ID_A(ID_A),
        .ID_B(ID_B),
        .jmp_or_not(jmp_or_not)
    );

    Add_32   add_32_T2                   //执行pc+imm
    (
         .former(ID_PC),
         .addend(ID_IMM),
         .latter(ID_ADDER)        
    );

    Add_32  add_32_T5                    //执行pc+4
    (           
        .former(ID_A),
        .addend(ID_IMM),
        .latter(PC_jalr)
    );

    
    REG_IDEX idex
    (
        .clk(clk),
        .rst(rst),
        .ID_PC(ID_PC),
        .ID_IR(ID_IR),
        .ID_A(ID_A),
        .ID_B(ID_B),
        .ID_rs1(ID_rs1),
        .ID_rs2(ID_rs2),
        .ID_rd(ID_rd),
        .ID_IMM(ID_IMM),
        .ID_alu_op(ID_alu_op), .ID_pc_src(ID_pc_src), .ID_mem_to_reg(ID_mem_to_reg), .ID_reg_write(ID_reg_write), .ID_alu_src(ID_alu_src), .ID_branch(ID_branch), 
        .ID_b_type(ID_b_type), .ID_mem_read(ID_mem_read), .ID_mem_write(ID_mem_write), .ID_b_type2(ID_b_type2), .ID_LBinReg(ID_LBinReg), .ID_LBinRam(ID_LBinRam),
        .ID_CSR_read_data(ID_CSR_read_data),
        .ID_jmp_or_not(jmp_or_not),
        .ID_ADDER(ID_ADDER),
        .EX_PC(EX_PC),
        .EX_IR(EX_IR),
        .EX_A(EX_A),
        .EX_B(EX_B),
        .EX_rs1(EX_rs1),
        .EX_rs2(EX_rs2),
        .EX_rd(EX_rd),
        .EX_IMM(EX_IMM),
        .EX_alu_op(EX_alu_op), .EX_pc_src(EX_pc_src), .EX_mem_to_reg(EX_mem_to_reg), .EX_reg_write(EX_reg_write), .EX_alu_src(EX_alu_src), .EX_branch(EX_branch), 
        .EX_b_type(EX_b_type), .EX_mem_read(EX_mem_read), .EX_mem_write(EX_mem_write), .EX_b_type2(EX_b_type2), .EX_LBinReg(EX_LBinReg), .EX_LBinRam(EX_LBinRam),
        .EX_CSR_read_data(EX_CSR_read_data),
        .EX_jmp_or_not(EX_jmp_or_not),
        .EX_ADDER(EX_ADDER)
    );
   
   
   
    //EX

    FORWARD forwarding
    (
        .EX_rs1(EX_rs1),
        .EX_rs2(EX_rs2),
        .EX_IR(EX_IR),
        .MEM_rd(MEM_rd),
        .WB_rd(WB_rd),
        .WB_IR(WB_IR),
        .MEM_IR(MEM_IR),
        .CSR_forward_a(CSR_forward_a),
        .CSR_forward_b(CSR_forward_b),
        .MEM_reg_write(MEM_reg_write),
        .WB_reg_write(WB_reg_write),
        .forward_a(forward_a),
        .forward_b(forward_b),
        .EX_alu_src(EX_alu_src)
    );

    MUX3_1T1 mux3_1_1
    (
        .T0(EX_B),
        .T1(WB_data),
        .T2(MEM_ALUout),
        .T3(MEM_IMM),
        .s(forward_b),
        .csr(CSR_forward_b),
        .csr_read(MEM_CSR_read_data),
        .mem_to_reg(MEM_mem_to_reg),
        .o(final_B)
    );

    MUX3_1T1 mux3_1_2
    (
        .T0(EX_A),
        .T1(WB_data),
        .T2(MEM_ALUout),
        .T3(MEM_IMM),
        .s(forward_a),
        .csr(CSR_forward_a),
        .csr_read(MEM_CSR_read_data),
        .mem_to_reg(MEM_mem_to_reg),
        .o(final_A)
    );



   MUX2T1_32   mux2_0                   //二路选择器，s为1输出T1
    (
        .T0(final_B),
        .T1(EX_IMM),
        .s(EX_alu_src),                  //1'b1表示ALU B口的操作源来自imm，1'b0表示数据来自Reg[rs2]
        .o(read_data_3)
    );
    ALU     alu_operation
    (
        .a(final_A),
        .b(read_data_3),
        .alu_op(EX_alu_op),
        .res(EX_ALUout),
        .zero(EX_ALUzero)
    );
    

    REG_EXMEM exmem 
    (
        .clk(clk), .rst(rst), .EX_A(EX_A),
        .EX_rd(EX_rd),
        .EX_PC(EX_PC), .EX_ALUzero(EX_ALUzero), .EX_ALUout(EX_ALUout), .EX_B(final_B), .EX_IR(EX_IR), .EX_IMM(EX_IMM),
        .EX_pc_src(EX_pc_src), .EX_mem_to_reg(EX_mem_to_reg), .EX_reg_write(EX_reg_write), .EX_branch(EX_branch), .EX_b_type(EX_b_type), .EX_b_type2(EX_b_type2),
        .EX_mem_read(EX_mem_read), .EX_mem_write(EX_mem_write), .EX_LBinReg(EX_LBinReg), .EX_LBinRam(EX_LBinRam),
        .EX_CSR_read_data(EX_CSR_read_data),
        .MEM_PC(MEM_PC), .MEM_ALUzero(MEM_ALUzero), .MEM_ALUout(MEM_ALUout), .MEM_B(MEM_B), .MEM_IR(MEM_IR), .MEM_IMM(MEM_IMM),
        .MEM_pc_src(MEM_pc_src), .MEM_mem_to_reg(MEM_mem_to_reg), .MEM_reg_write(MEM_reg_write), .MEM_branch(MEM_branch), .MEM_b_type(MEM_b_type), .MEM_b_type2(MEM_b_type2),
        .MEM_mem_read(MEM_mem_read), .MEM_mem_write(MEM_mem_write), .MEM_A(MEM_A), .MEM_LBinReg(MEM_LBinReg), .MEM_LBinRam(LBinRam),
        .MEM_rd(MEM_rd),
        .mem_write(mem_write),
        .addr_out(addr_out),
        .data_out(data_out),
        .data_in(data_in),
        .MEM_readdata(MEM_readdata),
        .MEM_CSR_read_data(MEM_CSR_read_data_temp)
    );

    
    
    
   
    
    //MEM
    
    Forward_for_CSR forwardcsr (
        .MEM_IR(MEM_IR),
        .WB_rd(WB_rd),
        .WB_data(WB_data),
        .WB_reg_write(WB_reg_write),
        .MEM_CSR_read_data(MEM_CSR_read_data),
        .MEM_CSR_read_data_temp(MEM_CSR_read_data_temp)
    );

    REG_MEMWB memwb 
    (
        .clk(clk), .rst(rst), .MEM_IMM(MEM_IMM), .WB_IMM(WB_IMM), .MEM_PC(MEM_PC), .WB_PC(WB_PC),
        .MEM_rd(MEM_rd), .MEM_mem_read(MEM_mem_read), .MEM_LBinReg(MEM_LBinReg),
        .MEM_readdata(MEM_readdata), .MEM_ALUout(MEM_ALUout), .MEM_PCadd4(PCadd4), .MEM_IR(MEM_IR), .MEM_mem_to_reg(MEM_mem_to_reg), .MEM_reg_write(MEM_reg_write),
        .MEM_CSR_read_data(MEM_CSR_read_data),
        .WB_readdata(WB_readdata), .WB_ALUout(WB_ALUout), .WB_PCadd4(WB_PCadd4), .WB_IR(WB_IR), .WB_mem_to_reg(WB_mem_to_reg), .WB_reg_write(WB_reg_write),
        .WB_rd(WB_rd), .WB_mem_read(WB_mem_read), .WB_LBinReg(WB_LBinReg),
        .WB_CSR_read_data(WB_CSR_read_data)
    );
    
    //WB
    // 2'b00 表示写回rd的数据来自ALU, 2'b01表示数据来自imm, 2'b10表示数据来自pc+4, 2'b11 表示数据来自data memory
    
    
    Add_32  add_32_T4                    //执行pc+4
    (           
        .former(WB_PC),
        .addend(64'd4),
        .latter(PC_to_reg)
    );

    MUX_WB muxwb (WB_mem_to_reg, WB_ALUout, PC_to_reg, MEM_readdata, WB_IMM, WB_data_temp);
    MUX_WB2 muxwb2 (
        .WB_IR(WB_IR),
        .WB_data_temp(WB_data_temp),
        .WB_CSR_read_data(WB_CSR_read_data),
        .WB_data(WB_data)
    );
    

endmodule

module MUX_WB2(
    input [31:0] WB_IR,
    input [63:0] WB_data_temp,
    input [63:0] WB_CSR_read_data,
    output reg [63:0] WB_data
    );
    always @(*)
    begin
        if(WB_IR[6:0]==7'b1110011 && WB_IR[14:12]!=3'b000 && WB_IR[14:12]!=3'b100 )  //特权指令 且非mret ecall 和 ...
            WB_data = WB_CSR_read_data;
        else
            WB_data = WB_data_temp;
    end
endmodule







module REG_IFID(
    input clk,
    input rst,
    input [63:0] IF_PC,
    input [31:0] IF_IR,
    input  IF_predict_jmp,
    input IF_if_foundBTB,
    input [63:0] IF_predict_jmp_PC,
    output reg [63:0] ID_PC,
    output reg [31:0] ID_IR,
    output reg   ID_predict_jmp,
    output reg ID_if_foundBTB,
    output reg [63:0] ID_predict_jmp_PC
    );
    always@(posedge clk or posedge rst)
    begin
        if (rst == 1) 
        begin
            ID_PC <= 64'b0;
            ID_IR <= 32'b0;
            ID_predict_jmp <= 0;
            ID_if_foundBTB <= 0;
            ID_predict_jmp_PC <= 64'b0;
        end
        else 
        begin
            ID_PC <= IF_PC;
            ID_IR <= IF_IR;
            ID_predict_jmp <= IF_predict_jmp;
            ID_if_foundBTB <= IF_if_foundBTB;
            ID_predict_jmp_PC <= IF_predict_jmp_PC;
        end
    end
endmodule


module Control(
    input   [6:0]   op_code,
    input   [2:0]   funct3,
    input   funct7_5,
    output  reg  [1:0]   pc_src,         
    output  reg reg_write,   
    output  reg alu_src_b,   
    output  reg [3:0]   alu_op,        
    output  reg [1:0]   mem_to_reg, 
    output  reg mem_write,   
    output  reg mem_read,
    output  reg branch,         
    output  reg b_type,
    output  reg ill_instr,
    output  reg b_type3,
    output  reg [1:0] b_type2,
    output  reg [1:0] LBinReg,
    output  reg [1:0] LBinRam
    );
    
    reg     [1:0]   aluop;
    //reg     ill_instr;
    wire    [3:0]   funt;
    assign  funt =   {funct3, funct7_5};
    always  @ (*)  begin
        alu_src_b   =   0;
        mem_to_reg  =   0;
        mem_write   =   0;
        mem_read    =   0;
        reg_write   =   0;
        branch      =   0;
        b_type      =   0;
        aluop       =   2'b10;
        pc_src      =   0;
        ill_instr   =   0;
        b_type3     =   0;
        b_type2     =   2'b00;
        LBinReg     =   2'b00;
        LBinRam     =   2'b00;
        case(op_code)
        
            7'b0010011:
                begin
                    case(funct3)
                        3'b000:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //ADDI
                        3'b010:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0010; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //SLTI
                        3'b111:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0111; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //ANDI
                        3'b110:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0110; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //ORI
                        3'b101:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0101; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //SRLI(????????)
                        3'b001:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0001; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //SLLI(????????)
                        3'b100: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0100; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end     //XORI

                        default: begin pc_src = 2'b00; reg_write = 0; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 0; mem_write = 0; branch = 0; b_type = 0; ill_instr = 1; end
                    endcase
                end
            
            7'b0011011:
                begin
                    case(funct3)
                       3'b000: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0;LBinReg = 2'b10; end      //ADDIW
                       3'b001: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0001; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; LBinReg = 2'b10; end      //SLLIW
                       //3'b000:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //ADDI
                        //3'b010:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0010; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //SLTI
                       default: begin pc_src = 2'b00; reg_write = 0; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 0; mem_write = 0; branch = 0; b_type = 0; ill_instr = 1; LBinReg = 2'b00; end
                   endcase
                end
            7'b0110111:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0001; mem_to_reg = 2'b01; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end  //LUI
            7'b0010111:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0001; mem_to_reg = 2'b01; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end  //auipc
            7'b1100111:begin branch=0; pc_src=2'b01; mem_to_reg=2'b10; reg_write=1; mem_write=0; alu_src_b=0; b_type=0; alu_op=0;ill_instr = 0; mem_to_reg = 2'b10; end   //jalr
            7'b0110011:
                begin
                    case(funct3)
                        3'b000: if(funct7_5==0) begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end     //ADD
                        else begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b1000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end //sub
                        3'b010: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b0010; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end    //SLT
                        3'b111: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b0111; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end    //AND
                        3'b110: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b0110; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end    //OR
                        3'b001: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b0001; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end    //SLL
                        3'b101:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b1101; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end      //SRL
                        default: begin pc_src = 2'b00; reg_write = 0; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 0; mem_write = 0; branch = 0; b_type = 0; ill_instr = 1; end
                    endcase
                end
            7'b1101111:begin pc_src = 2'b10; reg_write = 1; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b10; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; end              //JAL
                       
            7'b1100011:
                begin
                    case(funct3)
                        3'b000: begin pc_src = 2'b10; reg_write = 0; alu_src_b = 0; alu_op = 4'b1000; mem_to_reg = 0; mem_write = 0; branch = 1; b_type = 1; ill_instr = 0; end    //BEQ
                        3'b001: begin pc_src = 2'b10; reg_write = 0; alu_src_b = 0; alu_op = 4'b1000; mem_to_reg = 0; mem_write = 0; branch = 1; b_type = 0; ill_instr = 0; end    //BNE
                        3'b111: begin pc_src = 2'b10; reg_write = 0; alu_src_b = 0; alu_op = 4'b1000; mem_to_reg = 0; mem_write = 0; branch = 1; b_type = 0; ill_instr = 0; b_type2 = 2'b10; end //BGEU
                        3'b101: begin pc_src = 2'b10; reg_write = 0; alu_src_b = 0; alu_op = 4'b1000; mem_to_reg = 0; mem_write = 0; branch = 1; b_type = 0; ill_instr = 0; b_type2 = 2'b01; end //BGE
                        3'b100: begin pc_src = 2'b10; reg_write = 0; alu_src_b = 0; alu_op = 4'b1000; mem_to_reg = 0; mem_write = 0; branch = 1; b_type = 0; ill_instr = 0; b_type2 = 2'b11; b_type3 = 0; end //BLT
                        3'b110: begin pc_src = 2'b10; reg_write = 0; alu_src_b = 0; alu_op = 4'b1000; mem_to_reg = 0; mem_write = 0; branch = 1; b_type = 0; ill_instr = 0; b_type2 = 2'b11; b_type3 = 1; end //BLTU
                        default:begin pc_src = 2'b00; reg_write = 0; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 0; mem_write = 0; branch = 0; b_type = 0; ill_instr = 1; b_type2 = 2'b00; b_type3 = 0; end
                    endcase
                end
            7'b0000011:
                begin
                    case(funct3)
                        3'b010: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b11; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; mem_read = 1; LBinReg = 2'b10; end            //LW
                        3'b011: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b11; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; mem_read = 1; LBinReg = 2'b00; end            //LD
                        3'b100: begin pc_src = 2'b00; reg_write = 1; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b11; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; mem_read = 1; LBinReg = 2'b01; end            //LBU
                        default:begin pc_src = 2'b00; reg_write = 0; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 1; mem_read = 0; LBinReg = 2'b00; end            
                    endcase
                end
            7'b0100011:
                begin
                    case(funct3)
                        3'b010: begin pc_src = 2'b00; reg_write = 0; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 1; branch = 0; b_type = 0; ill_instr = 0; LBinRam = 2'b10; end            //SW
                        3'b011: begin pc_src = 2'b00; reg_write = 0; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 1; branch = 0; b_type = 0; ill_instr = 0; LBinRam = 2'b00; end            //SD
                        3'b000: begin pc_src = 2'b00; reg_write = 0; alu_src_b = 1; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 1; branch = 0; b_type = 0; ill_instr = 0; LBinRam = 2'b01; end            //SB
                        default:begin pc_src = 2'b00; reg_write = 0; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 1; LBinRam = 2'b00; end            
                    endcase
                end
            7'b0111011:begin pc_src = 2'b00; reg_write = 1; alu_src_b = 0; alu_op = 4'b0000; mem_to_reg = 2'b00; mem_write = 0; branch = 0; b_type = 0; ill_instr = 0; LBinReg = 2'b10; end                                      //ADDW
            
            endcase   
    end
    
endmodule



module Regs(
    input clk,
    input rst,
    input   [1:0]   LBinReg,
    input   [4:0]   read_addr_1,
    input   [4:0]   read_addr_2,
    output  [63:0]  read_data_1,
    output  [63:0]  read_data_2,
    input   we,     //写使能信号
    input   [4:0]   write_addr,
    input   [63:0]  write_data,
    output [63:0]ra,
    output [63:0]sp,
    output [63:0]gp,
    output [63:0]tp,
    output [63:0]t0,
    output [63:0]t1,
    output [63:0]t2,
    output [63:0]s0,
    output [63:0]s1,
    output [63:0]a0,
    output [63:0]a1,
    output [63:0]a2,
    output [63:0]a3,
    output [63:0]a4,
    output [63:0]a5,
    output [63:0]a6,
    output [63:0]a7,
    output [63:0]s2,
    output [63:0]s3,
    output [63:0]s4,
    output [63:0]s5,
    output [63:0]s6,
    output [63:0]s7,
    output [63:0]s8,
    output [63:0]s9,
    output [63:0]s10,
    output [63:0]s11,
    output [63:0]t3,
    output [63:0]t4,
    output [63:0]t5,
    output [63:0]t6
    
    );
    
    integer i;
    reg [63:0]  register [1:31];
    
    assign  read_data_1 = (read_addr_1 == 0) ? 0 : register[read_addr_1];   //取出rs1中的数字
    assign  read_data_2 = (read_addr_2 == 0) ? 0 : register[read_addr_2];   //取出rs2中的数字
    
    always @(posedge rst or negedge clk) 
            begin
                if (rst == 1) for (i = 1; i < 64; i = i + 1) register[i] <= 0; // reset
                else if (we !=0 && write_addr != 0) 
                    begin 
                        if(LBinReg == 2'b00)       begin register[write_addr] <= write_data; end
                        else if (LBinReg == 2'b10) begin register[write_addr][31:0] <= write_data[31:0]; end
                        else if (LBinReg == 2'b01) begin register[write_addr][7:0]  <= write_data[7:0];  end
                        else begin register[write_addr] <= register[write_addr]; end
                    end
            end
    
    assign ra = register[1];
    assign sp = register[2];
    assign gp = register[3];
    assign tp = register[4];
    assign t0 = register[5];
    assign t1 = register[6];
    assign t2 = register[7];
    assign s0 = register[8];
    assign s1 = register[9];
    assign a0 = register[10];
    assign a1 = register[11];
    assign a2 = register[12];
    assign a3 = register[13];
    assign a4 = register[14];
    assign a5 = register[15];
    assign a6 = register[16];
    assign a7 = register[17];
    assign s2 = register[18];
    assign s3 = register[19];
    assign s4 = register[20];
    assign s5 = register[21];
    assign s6 = register[22];
    assign s7 = register[23];
    assign s8 = register[24];
    assign s9 = register[25];
    assign s10 = register[26];
    assign s11 = register[27];
    assign t3 = register[28];
    assign t4 = register[29];
    assign t5 = register[30];
    assign t6 = register[31];
endmodule

module CSRregs(
    input clk,
    input rst,
    input ill_or_not,
    input [31:0] ID_IR,
    input [63:0] CSR_write_data,       //写入的CSR寄存器中的值  gpr[rs1]
    input [63:0] now_pc,
    output reg [63:0] CSR_read_data,       //读出的CSR寄存器中的值   gpr[rd]
    output reg [63:0] change_pc
    );
    reg [63:0] mepc;
    reg [63:0] mtvec;
    reg [63:0] mstatus;
    reg [63:0] mcause;
    reg [63:0] satp;
    initial
    begin
        CSR_read_data = 64'b0;
        change_pc = 64'b0;
    end

    always @(*)                         //read
    begin
        if(ill_or_not==1)
        begin
            change_pc = mtvec;
        end
        if(ID_IR[6:0]==7'b1110011)
        begin
            if(ID_IR[14:12]==3'b001)
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    CSR_read_data=mepc;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    CSR_read_data=mtvec;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    CSR_read_data=mstatus;
                end
                else if(ID_IR[31:20]==12'b000101000010) // scause
                begin
                    CSR_read_data=mcause;
                end
                else if(ID_IR[31:20]==12'b000110000000) //satp
                begin
                    CSR_read_data=satp;
                end
            end
            else if(ID_IR[14:12]==3'b010)
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    CSR_read_data=mepc;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    CSR_read_data=mtvec;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    CSR_read_data=mstatus;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    CSR_read_data=mcause;
                end
                else if(ID_IR[31:20]==12'b000110000000) //satp
                begin
                    CSR_read_data=satp;
                end
            end
            else if(ID_IR[14:12]==3'b011)
            begin
                if(ID_IR[31:20]==12'b000101000001)  //sepc
                begin
                    CSR_read_data=mepc;
                end
                else if(ID_IR[31:20]==12'b000100000101)  //stvec
                begin
                    CSR_read_data=mtvec;
                end
                else if(ID_IR[31:20]==12'b000100000000)  //sstatus
                begin
                    CSR_read_data=mstatus;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    CSR_read_data=mcause;
                end
                else if(ID_IR[31:20]==12'b000110000000) //satp
                begin
                    CSR_read_data=satp;
                end                
            end
            else if(ID_IR[14:12]==3'b101)
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    CSR_read_data=mepc;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    CSR_read_data=mtvec;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    CSR_read_data=mstatus;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    CSR_read_data=mcause;
                end
                else if(ID_IR[31:20]==12'b000110000000) //satp
                begin
                    CSR_read_data=satp;
                end
            end
            else if(ID_IR[14:12]==3'b110)
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    CSR_read_data=mepc;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    CSR_read_data=mtvec;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    CSR_read_data=mstatus;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    CSR_read_data=mcause;
                end
                else if(ID_IR[31:20]==12'b000110000000) //satp
                begin
                    CSR_read_data=satp;
                end
            end
            else if(ID_IR[14:12]==3'b111)
            begin
                if(ID_IR[31:20]==12'b000101000001)  //sepc
                begin
                    CSR_read_data=mepc;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    CSR_read_data=mtvec;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    CSR_read_data=mstatus;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    CSR_read_data=mcause;
                end
                else if(ID_IR[31:20]==12'b000110000000) //satp
                begin
                    CSR_read_data=satp;
                end
            end
            else if(ID_IR[14:12]==3'b000)
            begin
                if(ID_IR==32'h10200073)    //sret
                begin
                    change_pc=mepc;
                end
                else if(ID_IR==32'h00000073)  //ecall
                begin
                    change_pc=mtvec;
                end
            end
        end
    end

    always @(posedge rst or negedge clk)
    begin
        if(rst==1)
        begin
            mepc <= 64'b0;
            mtvec <= 64'b0;
            mstatus <= 64'b0;
            mcause <= 64'b0;
            CSR_read_data <= 64'b0;
            change_pc <= 64'b0;
        end
        else if(ID_IR[6:0]==7'b1110011)
        begin
            if(ill_or_not==1)
            begin
                    mstatus[3] <= mstatus[7];
                    mstatus[7] <= 1;
                    mcause[63] <= 0;
                    mcause[62:0] <= 63'h2;
                    mepc <= now_pc;
            end
            if(ID_IR[14:12]==3'b001)           //csrrw
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    mepc <= CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    mtvec <= CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    mstatus <= CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    mcause <= CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000110000000) //satp
                begin
                    satp <= CSR_write_data;
                end
            end
            else if(ID_IR[14:12]==3'b010)    //csrrs
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    mepc <= mepc | CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    mtvec <= mtvec | CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    mstatus <= mstatus | CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    mcause <= mcause | CSR_write_data;
                end
            end
            else if(ID_IR[14:12]==3'b011)    //csrrc
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    mepc <= mepc & ~CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    mtvec <= mtvec & ~CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    mstatus <= mstatus & ~CSR_write_data;
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    mcause <= mcause & ~CSR_write_data;
                end
            end
            else if(ID_IR[14:12]==3'b101)    //csrrwi
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    mepc <= ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    mtvec <= ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    mstatus <= ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    mcause <= ID_IR[19:15];
                end
            end
            else if(ID_IR[14:12]==3'b110)   //csrrsi
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    mepc=mepc | ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    mtvec=mtvec | ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    mstatus=mstatus | ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    mcause=mcause | ID_IR[19:15];
                end
            end
            else if(ID_IR[14:12]==3'b111)   //csrrci
            begin
                if(ID_IR[31:20]==12'b000101000001) //sepc
                begin
                    mepc=mepc & ~ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000100000101) //stvec
                begin
                    mtvec=mtvec & ~ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000100000000) //sstatus
                begin
                    mstatus=mstatus & ~ID_IR[19:15];
                end
                else if(ID_IR[31:20]==12'b000101000010) //scause
                begin
                    mcause=mcause & ~ID_IR[19:15];
                end
            end
            else if(ID_IR[14:12]==3'b000)  
            begin
                if(ID_IR==32'h10200073)    //sret
                begin
                    mstatus[7]=mstatus[3];
                    mstatus[3]=0;
                end
                else if(ID_IR==32'h00000073)  //ecall
                begin
                    mstatus[3]=mstatus[7];
                    mstatus[7]=1;
                    mcause[63]=0;
                    mcause[62:0]=63'd11;
                    mepc=now_pc;
                end
            end
        end
    end

endmodule



module ImmGen(
    input   [31:0]      instr,
    input   [63:0]      pc,
    output  reg [63:0]  Imm_out
    );
     
    wire    [6:0]   opcode;
    assign  opcode  = instr[6:0];
     
    always @ (*) begin
         case(opcode)
             7'b0010011 : Imm_out  = {{53{instr[31]}},instr[30:20]};  //I型指令
             7'b0011011 : Imm_out  = {{53{instr[31]}},instr[30:20]}; 
             7'b0110111 : Imm_out  = {32'b0, instr[31:12], 12'b0};                                          //U型指令
             7'b0010111 : Imm_out  = {32'b0, instr[31:12], 12'b0} + pc;                                     //auipc
             7'b0100011 : Imm_out  = {{53{instr[31]}}, instr[30:25], instr[11:7]};                   //S型指令
             //7'b0000011 : Imm_out  = {instr[31:20]};  
             7'b0000011 : begin Imm_out  = {{53{instr[31]}},instr[30:20]}; end                                             //LW
             7'b1100011 : Imm_out  = {{52{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};   //B型指令
             7'b1101111 : Imm_out  = {{44{instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0}; //J型指令
             default    : Imm_out  = 64'd0;                                              
         endcase
     end
endmodule


module REG_IDEX(
    input clk, rst,
    input [31:0] ID_IR,
    input [63:0] ID_PC, ID_A, ID_B, ID_IMM, ID_CSR_read_data, ID_ADDER,
    input [4:0] ID_rs1, ID_rs2, ID_rd,
    input [3:0] ID_alu_op,
    input [1:0] ID_pc_src, ID_mem_to_reg,
    input  ID_reg_write, ID_alu_src, ID_branch, ID_b_type, ID_mem_read, ID_mem_write,
    input [1:0] ID_b_type2,
    input [1:0] ID_LBinReg, ID_LBinRam,
    input ID_jmp_or_not,
    output reg [31:0] EX_IR,
    output reg [63:0] EX_PC, EX_A, EX_B, EX_IMM, EX_CSR_read_data, EX_ADDER,
    output reg [4:0] EX_rs1, EX_rs2, EX_rd,
    output reg [3:0] EX_alu_op,
    output reg [1:0] EX_pc_src, EX_mem_to_reg,
    output reg EX_reg_write, EX_alu_src, EX_branch, EX_b_type, EX_mem_read, EX_mem_write,
    output reg [1:0] EX_b_type2,
    output reg [1:0] EX_LBinReg, EX_LBinRam,
    output reg EX_jmp_or_not
    );
    always@(posedge clk or posedge rst)
    begin
        if (rst == 1) 
        begin
            EX_PC = 64'b0;
            EX_IR = 32'b0;
            EX_A  = 64'b0;
            EX_B  = 64'b0;
            EX_rs1 = 4'b0;
            EX_rs2 = 4'b0;
            EX_rd = 4'b0;
            EX_IMM = 64'b0;
            EX_alu_op = 4'b0;
            EX_pc_src = 2'b0;
            EX_mem_to_reg = 2'b0;
            EX_reg_write = 1'b0;
            EX_alu_src = 1'b0;
            EX_branch = 1'b0;
            EX_b_type = 1'b0;
            EX_mem_read = 1'b0;
            EX_mem_write = 1'b0;
            EX_CSR_read_data = 64'b0;
            EX_b_type2 = 2'b00;
            EX_LBinReg = 2'b00;
            EX_LBinRam = 2'b00;
            EX_jmp_or_not = 0;
            EX_ADDER = 64'b0;
        end
        else 
        begin
            EX_PC = ID_PC;
            EX_IR = ID_IR;
            EX_A  = ID_A;
            EX_B  = ID_B;
            EX_rs1 = ID_rs1;
            EX_rs2 = ID_rs2;
            EX_rd = ID_rd;
            EX_IMM = ID_IMM;
            EX_alu_op = ID_alu_op;
            EX_pc_src = ID_pc_src;
            EX_mem_to_reg = ID_mem_to_reg;
            EX_reg_write = ID_reg_write;
            EX_alu_src = ID_alu_src;
            EX_branch = ID_branch;
            EX_b_type = ID_b_type;
            EX_mem_read = ID_mem_read;
            EX_mem_write = ID_mem_write;
            EX_CSR_read_data = ID_CSR_read_data;
            EX_b_type2 = ID_b_type2;
            EX_LBinReg = ID_LBinReg;
            EX_LBinRam = ID_LBinRam;
            EX_jmp_or_not = ID_jmp_or_not;
            EX_ADDER = ID_ADDER;
        end
    end
endmodule


module MUX2T1_32(                             //2选1多路选择器，选择pc来源
     input   [63:0]  T0,
     input   [63:0]  T1,
     input   s,
     output  [63:0]  o
     );
     
     assign  o = s ? T1 : T0;
endmodule


module ALU(
    input   [63:0]  a,
    input   [63:0]  b,
    input   [3:0]    alu_op,
    output  reg [63:0]  res,
    output  reg zero            //结果为0，则zero输出高电平
    );
    `include "AluOp.vh"
    
    always@(*)begin
        case(alu_op)
            AND: res = a & b;  //and
            OR: res = a | b;  //or
            ADD: res = a + b;  //add
            SUB: res = a - b;  //sub
            SRL: res = a >> b; //srl
            XOR: res = a ^ b;  //xor
            SLT: res = ($signed(a) < $signed(b));  //slt 0010
            SLTU: res = (a < b);  //sltu 0011
            SRA: res = a >>> b;  //sra
            SLL: res = a << b;  //sll
            default: res = 64'hFFFFFFFFFFFFFFFF;
        endcase
        zero= (res == 0) ? 1 : 0;
    end
endmodule


module Add_32(
    input   [63:0]  former,
    input   [63:0]  addend,
    output  [63:0]  latter
    );
    
    assign  latter = former + addend;
endmodule


module REG_EXMEM(
    input clk, rst,
    input [31:0] EX_IR,
    input [63:0] EX_PC, EX_B, EX_ALUout, EX_IMM, EX_CSR_read_data,
    input [4:0] EX_rd,
    input [1:0] EX_pc_src, EX_mem_to_reg,
    input EX_reg_write, EX_branch, EX_b_type, EX_mem_read, EX_mem_write, EX_ALUzero,
    input [1:0] EX_b_type2,
    input [1:0] EX_LBinReg, EX_LBinRam,
    output reg [31:0] MEM_IR,
    output reg [63:0] MEM_PC, MEM_B, MEM_ALUout, MEM_IMM,
    output reg [1:0] MEM_pc_src, MEM_mem_to_reg,
    input [63:0] data_in,
    output reg [63:0] data_out, addr_out,
    output reg [4:0] MEM_rd,
    output reg mem_write,
    output reg [63:0] MEM_readdata, MEM_CSR_read_data,
    input [63:0] EX_A,
    output reg [63:0] MEM_A,

    output reg MEM_reg_write, MEM_branch, MEM_b_type, MEM_mem_read, MEM_mem_write, MEM_ALUzero,
    output reg [1:0] MEM_b_type2,
    output reg [1:0] MEM_LBinReg, MEM_LBinRam
    );
    always@(posedge clk or posedge rst)
    begin
        if (rst == 1) 
        begin
            MEM_IMM = 64'b0;
            MEM_PC = 64'b0;
            MEM_IR = 32'b0;
            MEM_B  = 64'b0;
            MEM_pc_src <= 2'b0;
            MEM_mem_to_reg <= 2'b0;
            MEM_ALUout <= 64'b0;
            MEM_reg_write <= 1'b0;
            MEM_ALUzero <= 1'b0;
            MEM_branch <= 1'b0;
            MEM_b_type <=1'b0;
            MEM_mem_read <= 1'b0;
            MEM_mem_write <= 1'b0;
            MEM_rd <= 4'b0;
            MEM_CSR_read_data <= 64'b0;
            MEM_b_type2 <= 2'b0;
            MEM_LBinReg <= 2'b00;
            MEM_LBinRam <= 2'b00;
        end
        else 
        begin
            MEM_A <= EX_A;
            MEM_IMM <= EX_IMM;
            MEM_PC <= EX_PC;
            MEM_IR <= EX_IR;
            MEM_B  <= EX_B;
            MEM_ALUout = EX_ALUout;
            MEM_pc_src <= EX_pc_src;
            MEM_mem_to_reg <= EX_mem_to_reg;
            MEM_reg_write <= EX_reg_write;
            MEM_ALUzero <= EX_ALUzero;
            MEM_branch <= EX_branch;
            MEM_b_type <= EX_b_type;
            MEM_mem_read <= EX_mem_read;
            MEM_mem_write <= EX_mem_write;
            MEM_rd <= EX_rd;
            data_out = EX_B;
            addr_out = MEM_ALUout;
            MEM_readdata = data_in;
            mem_write = EX_mem_write;
            MEM_CSR_read_data = EX_CSR_read_data;
            MEM_b_type2 = EX_b_type2;
            MEM_LBinReg = EX_LBinReg;
            MEM_LBinRam = EX_LBinRam;

        end
    end
endmodule


module MUX4T1(
     input   [63:0]  T0,
     input   [63:0]  T1,
     input   [63:0]  T2,
     input   [63:0]  T3,
     input   [1:0]   Sel,
     output  reg [63:0] o  
     );
     always @(*) begin
         case(Sel)
             2'b00:o=T0;                 //表示写回rd的数据来自ALU
             2'b01:o=T1;                 //表示数据来自imm
             2'b10:o=T2;                 //表示数据来自pc+4
             2'b11:o=T3;                 //表示数据来自data memory
             default: o=64'b0;
         endcase
     end
endmodule


module REG_MEMWB(
    input clk, rst,
    input [31:0] MEM_IR,
    input [63:0] MEM_readdata, MEM_ALUout, MEM_PCadd4, MEM_IMM, MEM_CSR_read_data, MEM_PC,
    input [1:0] MEM_mem_to_reg,
    input [4:0] MEM_rd,
    input MEM_reg_write, MEM_mem_read,
    input [1:0] MEM_LBinReg,
    output reg [31:0] WB_IR,
    output reg [63:0] WB_readdata, WB_ALUout, WB_PCadd4, WB_IMM, WB_CSR_read_data, WB_PC,
    output reg [1:0] WB_mem_to_reg,
    output reg [4:0] WB_rd, WB_mem_read,
    output reg WB_reg_write,
    output reg [1:0] WB_LBinReg
    );
    always@(posedge clk or posedge rst)
    begin
        if (rst == 1) 
        begin
            WB_IMM <=64'b0;
            WB_IR <= 32'b0;
            WB_readdata <= 64'b0;
            WB_ALUout  <= 64'b0;
            WB_PCadd4 <= 64'b0;
            WB_mem_to_reg <= 2'b0;
            WB_reg_write <= 1'b0;
            WB_PC <=64'b0;
            WB_rd <= 4'b0;
            WB_mem_read <= 0;
            WB_CSR_read_data <= 64'b0;
            WB_LBinReg <= 2'b00;
        end
        else 
        begin
            WB_IMM <= MEM_IMM;
            WB_IR <= MEM_IR;
            WB_readdata <= MEM_readdata;
            WB_ALUout  <= MEM_ALUout;
            WB_PCadd4 <= MEM_PCadd4;
            WB_mem_to_reg <= MEM_mem_to_reg;
            WB_reg_write <= MEM_reg_write;
            WB_PC <= MEM_PC;
            WB_rd <= MEM_rd;
            WB_mem_read <= MEM_mem_read;
            WB_CSR_read_data <= MEM_CSR_read_data;
            WB_LBinReg = MEM_LBinReg;
        end
    end
endmodule



module MUX_WB(
    input [1:0] WB_mem_to_reg,
    input [63:0] WB_ALUout, WB_PCadd4, WB_readdata, WB_IMM,
    output reg [63:0] WB_data
    );
    always @(*)
    begin
        case(WB_mem_to_reg)
            2'b00: WB_data = WB_ALUout;
            2'b10: WB_data = WB_PCadd4;
            2'b01: WB_data = WB_IMM;
            2'b11: WB_data = WB_readdata;
            default: WB_data = 64'b0;
        endcase
    end
endmodule


module PC_reg(
    input clk,
    input CE,
    input PC_write,
    input rst,
    input ill_or_not,
    input [63:0] old,
    input [63:0] stall,
    input [31:0] ID_IR,
    input [63:0] CSR_pcsrc,
    output reg [63:0] new
    );
    
    always@(posedge clk or posedge rst)
    begin
    if (rst == 1) new = 64'b0;
    else if (CE == 1)
    begin
        if(PC_write==0)
        begin
            if(ID_IR==32'h00000073||ID_IR==32'h10200073|| ill_or_not==1)      //ecall or sret
                new = CSR_pcsrc;
            else
                new = old;
        end
        else if(PC_write==1)
            new = stall;
    end
    end
endmodule



module MUX3T1(
    input [31:0] T0,
    input [31:0] T1,
    input [31:0] T2,
    input [1:0] s,
    output reg [31:0] o
    );
    always @(*) begin
        case(s)
            2'b00:o=T0;
            2'b01:o=T1;
            2'b10:o=T2;
            default:o=32'b0;
        endcase
    end
endmodule



module FORWARD(
    input [4:0] EX_rs1, EX_rs2,
    input [4:0] MEM_rd, WB_rd,
    input MEM_reg_write, WB_reg_write,
    input EX_alu_src,
    input [31:0] EX_IR, MEM_IR, WB_IR,
    output reg [1:0] forward_a,
    output reg [1:0] forward_b,
    output reg [1:0] CSR_forward_a,
    output reg [1:0] CSR_forward_b
    );
    always @(*) begin
        forward_a = 2'b00;
        forward_b = 2'b00;
        CSR_forward_a = 2'b00;
        CSR_forward_b = 2'b00;
        if(WB_IR[6:0]==7'b1110011&&WB_rd!=0)
        begin
            if(WB_rd==EX_rs1)
                CSR_forward_a = 2'b01;
            if(WB_rd==EX_rs2)
                CSR_forward_b = 2'b01;
        end
        if(MEM_IR[6:0]==7'b1110011&&MEM_rd!=0)
        begin
            if(MEM_rd==EX_rs1)
                CSR_forward_a = 2'b10;
            if(MEM_rd==EX_rs2)
                CSR_forward_b = 2'b10;
        end
        if(WB_reg_write==1&&WB_rd!=0)
        begin
            if(WB_rd==EX_rs1)
                forward_a = 2'b01;
            if(WB_rd==EX_rs2&&EX_alu_src==0 || WB_rd==EX_rs2 && EX_IR[6:0]==7'b0100011)
                forward_b = 2'b01;
        end
        if(MEM_reg_write==1&&MEM_rd!=0)
        begin
            if(MEM_rd==EX_rs1)
                forward_a = 2'b10;
            if(MEM_rd==EX_rs2)
                forward_b = 2'b10;
        end

        
    end
endmodule


module Detection(
    input [4:0] ID_rs1, ID_rs2,
    input [4:0] EX_rd,
    input [4:0] ID_rd,
    input ID_mem_read,
    input ID_predict_jmp,
    input ID_if_foundBTB,
    input [31:0] IF_IR_in,
    input [31:0] ID_IR_in,
    input ill_or_not,
    input ID_branch,
    input EX_mem_read,
    input jmp_or_not,
    input [1:0] ID_mem_to_reg,
    output reg [31:0] IF_IR_out,
    output reg [31:0] ID_IR_out,
    output reg control_write,
    output reg PC_write
    );
    initial
    begin
        control_write <= 1'b0;
        PC_write <= 1'b0;
    end
    always @(*) begin
        
        
        /*if(ID_mem_read==1&&EX_rd!=4'b0&&(EX_rd==ID_rs1||EX_rd==ID_rs2))
        begin
            //control_write = 1;
            PC_write = 1;
            ID_IR_out = 8'h00000013;
        end*/
        if(ID_IR_in==32'h00000073)  //ecall or sret
        begin
            PC_write = 0;
            ID_IR_out=ID_IR_in;
            IF_IR_out=8'h00000013;
        end
        else if(ID_IR_in==32'h10200073 )
        begin
            PC_write = 0;
            ID_IR_out=ID_IR_in;
            IF_IR_out=8'h00000013;
        end
        else if(ill_or_not==1'b1)
        begin
            PC_write = 0;
            ID_IR_out=ID_IR_in;
            IF_IR_out=8'h00000013;
        end
        else if(EX_mem_read==1&&EX_rd!=0&&(IF_IR_in[19:15]==EX_rd||IF_IR_in[24:20]==EX_rd))
        begin
            PC_write = 1;
            ID_IR_out = ID_IR_in;
            IF_IR_out = 8'h00000013;
        end
        else if(ID_mem_read==1&&ID_rd!=0&&(IF_IR_in[19:15]==ID_rd||IF_IR_in[24:20]==ID_rd))             //load  use
            begin
                //control_write = 1;
                PC_write = 1;
                ID_IR_out = ID_IR_in;
                IF_IR_out = 8'h00000013;
            end
        else if(ID_IR_in[6:0]==7'b1101111 && ID_if_foundBTB == 0)
        begin
            PC_write = 0;
            ID_IR_out = ID_IR_in;
            IF_IR_out = 8'h00000013;
        end
        else if(ID_IR_in[6:0]==7'b1100111)           //jalr or ret
        begin
            PC_write = 0;
            ID_IR_out = ID_IR_in;
            IF_IR_out = 8'h00000013;
        end
        else if(ID_mem_to_reg==2'b10&&ID_IR_in[6:0]!=7'b1101111)             //load  use
            begin
                //control_write = 1;
                PC_write = 1;
                ID_IR_out = ID_IR_in;
                IF_IR_out = 8'h00000013;
            end
        else if(ID_branch==1)
        begin
            if(jmp_or_not == 1 && ID_predict_jmp == 0)                                                 //预测不跳转，实际跳转
            begin
                PC_write = 0;
                ID_IR_out = ID_IR_in;
                IF_IR_out = 8'h00000013;
            end
            else if(jmp_or_not == 0 && ID_predict_jmp == 1)                                         //预测跳转，实际不跳转
            begin
                PC_write = 0;
                ID_IR_out = ID_IR_in;
                IF_IR_out = 8'h00000013;
            end
            else if(EX_mem_read==1&&EX_rd!=0&&(EX_rd==ID_rs1||EX_rd==ID_rs2))             //load  use
            begin
                //control_write = 1;
                PC_write = 1;
                IF_IR_out = IF_IR_in;
                ID_IR_out = 8'h00000013;
            end
            else
            begin
                IF_IR_out = IF_IR_in;
                ID_IR_out = ID_IR_in;
                control_write = 0;
                PC_write = 0;
            end
        end
        
        else
        begin
            IF_IR_out = IF_IR_in;
            ID_IR_out = ID_IR_in;
            control_write = 0;
            PC_write = 0;
        end
    end
endmodule


module MUX2T1_control (
    input control_write,
    input [1:0] ID_pc_src_temp,
    input ID_reg_write_temp,
    input [3:0] ID_alu_op_temp,
    input ID_alu_src_temp,
    input [1:0] ID_mem_to_reg_temp,
    input ID_mem_read_temp, ID_mem_write_temp,
    input ID_branch_temp,
    input ID_b_type_temp,
    input ID_b_type3_temp,
    input [1:0] ID_b_type2_temp,
    input [1:0] ID_LBinReg_temp,
    input [1:0] ID_LBinRam_temp,
    input ill_instr_temp,
    output reg [1:0] ID_pc_src,
    output reg ID_reg_write,
    output reg [3:0] ID_alu_op,
    output reg ID_alu_src,
    output reg [1:0] ID_mem_to_reg,
    output reg ID_mem_read, ID_mem_write,
    output reg ID_branch,
    output reg ID_b_type,
    output reg ID_b_type3,
    output reg [1:0] ID_b_type2,
    output reg [1:0] ID_LBinReg,
    output reg [1:0] ID_LBinRam,
    output reg ill_instr
    );
    initial
    begin
        ill_instr = 1'b0;
    end
    always @(*) begin
        if(control_write==1)
        begin
            ID_pc_src = 2'b0;
            ID_reg_write = 1;
            ID_alu_op = 4'b0000;
            ID_alu_src = 0;
            ID_mem_to_reg = 2'b0;
            ID_mem_read = 0;
            ID_mem_write = 0;
            ID_branch = 0;
            ID_b_type = 0;
            ID_b_type2 = 2'b0;
            ID_b_type3 = 0;
            ID_LBinReg = 2'b0;
            ID_LBinRam = 2'b0;
            ill_instr  = 0;
        end
        else
        begin
            ID_pc_src = ID_pc_src_temp;
            ID_reg_write = ID_reg_write_temp;
            ID_alu_op = ID_alu_op_temp;
            ID_alu_src = ID_alu_src_temp;
            ID_mem_to_reg = ID_mem_to_reg_temp;
            ID_mem_read = ID_mem_read_temp;
            ID_mem_write = ID_mem_write_temp;
            ID_branch = ID_branch_temp;
            ID_b_type = ID_b_type_temp;
            ID_b_type2 = ID_b_type2_temp;
            ID_b_type3 = ID_b_type3_temp;
            ID_LBinReg = ID_LBinReg_temp;
            ID_LBinRam = ID_LBinRam_temp;
            ill_instr = ill_instr_temp;
        end
    end
endmodule



module Jmp_detection (
    input ID_branch,
    input ID_b_type,
    input [1:0] ID_b_type2,
    input ID_b_type3,
    input [63:0] ID_A,
    input [63:0] ID_B,
    output reg jmp_or_not
    );
    initial
    begin
        jmp_or_not <= 1'b0;
    end
    always @(*) 
    begin
        if(ID_branch==1)
        begin
            if(ID_b_type2==2'b0)
            begin
                if(ID_b_type==1 && ID_A==ID_B)
                    jmp_or_not = 1;
                else if(ID_b_type==0 && ID_A!=ID_B)
                    jmp_or_not = 1;
                else
                    jmp_or_not = 0;
            end
            else if(ID_b_type2==2'b01)              //bge
            begin
                if(ID_A>=ID_B)
                    jmp_or_not = 1;
                else
                    jmp_or_not = 0;
            end
            else if(ID_b_type2==2'b10)               //bgeu
            begin
                if(ID_A>=ID_B)
                    jmp_or_not = 1;
                else
                    jmp_or_not = 0;
            end
            else if(ID_b_type2==2'b11)               //blt and bltu  
            begin
                if(ID_b_type3==0)                   //blt
                begin
                    if(ID_A[63]^ID_B[63]==1'b0)     //  0 0  or  1 1
                    begin
                        if(ID_A<ID_B)
                            jmp_or_not = 1;
                        else
                            jmp_or_not = 0;
                    end
                    else                             //  1 0   or 0 1
                    begin
                        if(ID_A[63]==1'b0)
                            jmp_or_not = 0;
                        else
                            jmp_or_not = 1;
                    end
                end
                else                                //bltu
                begin
                    if(ID_A<ID_B)
                        jmp_or_not = 1;
                    else
                        jmp_or_not = 0;
                end
            end
        end
        else
            jmp_or_not = 0;
    end
endmodule


module Forward_for_B (
    input [31:0] EX_IR,
    input [31:0] ID_IR,
    input [31:0] MEM_IR,
    input [31:0] WB_IR,
    input ID_branch,
    input MEM_mem_read,
    input WB_mem_read,
    input [63:0] WB_readdata,
    input [4:0] ID_rs1, ID_rs2,
    input [4:0] EX_rd, MEM_rd, WB_rd,
    input [63:0] ID_A_temp, ID_B_temp,
    input EX_reg_write,
    input MEM_reg_write,
    input WB_reg_write,
    input EX_mem_read,
    input [63:0] WB_data,
    input [63:0] MEM_readdata,
    input [63:0] EX_ALUout, 
    input [63:0] MEM_ALUout,
    input [63:0] EX_CSR_read_data,
    input [63:0] MEM_CSR_read_data,
    output reg [63:0] ID_A, ID_B
    );
    always @(*) begin
        ID_A = ID_A_temp;
        ID_B = ID_B_temp;
        if(WB_reg_write==1&&WB_rd!=0 || WB_IR[6:0]==7'b1110011&&WB_rd!=0)
        begin
            if(WB_rd==ID_rs1)
                ID_A = WB_data;
            if(WB_rd==ID_rs2)
                ID_B = WB_data;
        end
        if(ID_branch==1||ID_IR[6:0]==7'b1110011||ID_IR[6:0]==7'b1100111)
        begin
            if(WB_mem_read==1&&WB_rd!=0)
            begin
                if(WB_rd==ID_rs1)
                    ID_A = WB_data;
                if(WB_rd==ID_rs2)
                    ID_B = WB_data;
            end
            if(MEM_reg_write==1&&MEM_mem_read==0&&MEM_rd!=0&&MEM_IR[6:0]!=7'b1101111)
            begin
                if(MEM_rd==ID_rs1)
                    ID_A = MEM_ALUout;
                if(MEM_rd==ID_rs2)
                    ID_B = MEM_ALUout;
            end
            if(EX_reg_write==1&&EX_mem_read==0&&EX_rd!=0&&EX_IR[6:0]!=7'b1101111)
            begin
                if(EX_rd==ID_rs1)
                    ID_A = EX_ALUout;
                if(EX_rd==ID_rs2)
                    ID_B = EX_ALUout;
            end
            
            if(EX_IR[6:0]==7'b1110011&&EX_rd!=0)
            begin
                if(EX_rd==ID_rs1)
                    ID_A = EX_CSR_read_data;
                if(EX_rd==ID_rs2)
                    ID_B = EX_CSR_read_data;
            end
            if(MEM_IR[6:0]==7'b1110011&&MEM_rd!=0)
            begin
                if(MEM_rd==ID_rs1)
                    ID_A = MEM_CSR_read_data;
                if(MEM_rd==ID_rs2)
                    ID_B = MEM_CSR_read_data;
            end
            if(WB_IR[6:0]==7'b1110011&&WB_rd!=0)
            begin
                if(WB_rd==ID_rs1)
                    ID_A = WB_data;
                if(WB_rd==ID_rs2)
                    ID_B = WB_data;
            end
        end
    end
endmodule


module MUX3_1T1(
    input [63:0] T1, T0,
    input [63:0] T2, T3,
    input [63:0] csr_read,
    input [1:0] s,
    input [1:0] csr,
    input [1:0] mem_to_reg,
    output reg [63:0] o
    );
    always @(*) begin
        if(s==2'b00 && csr==2'b00)
            o = T0;
        if(s==2'b01||csr==2'b01)
            o = T1;
        if(s==2'b10)
        begin
            if(mem_to_reg==2'b01)
                o = T3;
            else 
                o = T2;
        end
        if(csr==2'b10)
            o = csr_read;
    end
endmodule

module Forward_for_CSR (
    input [31:0] MEM_IR,
    input  WB_reg_write,
    input [63:0] WB_data,
    input [4:0] WB_rd,
    input [63:0] MEM_CSR_read_data_temp,
    output reg [63:0] MEM_CSR_read_data
    );
    initial
    begin
        MEM_CSR_read_data = 64'B0;
    end
    always @(*)
    begin
        if(MEM_IR[6:0]==7'b1110011&&WB_rd!=0&&MEM_IR[11:7]==WB_rd)
        begin
            MEM_CSR_read_data = WB_data;
        end
        else
        begin
            MEM_CSR_read_data = MEM_CSR_read_data_temp;
        end
    end
endmodule

module ill_detection (
    input [31:0] ID_IR,
    input ill_instr,
    output reg ill_or_not
    );
    initial 
    begin
        ill_or_not <= 1'b0;
    end
    always @(*)
    begin
        ill_or_not = 0;
        if(ID_IR[6:0]!=7'b1110011)
        begin
            if(ill_instr==1)
                ill_or_not=1;
        end
        else if(ID_IR[6:0]==7'b1110011&&ID_IR[14:12]!=3'b000)
        begin
            if(ID_IR[31:20]>=12'h800 && ID_IR[31:20]<=12'h8ff||ID_IR[31:20]==12'hc00)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'hcc0 && ID_IR[31:20]<=12'hcff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'h5c0 && ID_IR[31:20]<=12'h5ff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'h9c0 && ID_IR[31:20]<=12'h9ff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'hdc0 && ID_IR[31:20]<=12'hdff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'h6c0 && ID_IR[31:20]<=12'h6ff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'hac0 && ID_IR[31:20]<=12'haff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'hec0 && ID_IR[31:20]<=12'heff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'h7c0 && ID_IR[31:20]<=12'h7ff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'hbc0 && ID_IR[31:20]<=12'hbff)
                ill_or_not = 1;
            else if(ID_IR[31:20]>=12'hfc0 && ID_IR[31:20]<=12'hfff)
                ill_or_not = 1;
        end
    end
endmodule

module BHTtable (
    input rst,
    input clk,
    input [63:0] PC,
    input [31:0] IF_IR,
    input [63:0] add_PC,
    input if_foundBTB,
    input jmp_or_not,                           //ID阶段是否跳转
    input we,                                   //是否写入BHT表
    output reg predict_jmp
    );
    reg [13:0] BHT [1:256];                   // [13:2]表示PC的后12位，[1:0]表示预测跳转情况。
    integer i;
    integer find;
    initial 
    begin
        for(i = 1; i < 256; i = i + 1) BHT[i] <= 14'b0;
        predict_jmp <= 1'b0;
    end
    always @(*)
    begin
        if(if_foundBTB == 1'b0)
            predict_jmp = 0;
        else 
        begin
        i = 1;
        if(IF_IR[6:0] == 7'b1101111)
            predict_jmp = 1;
        else
        begin
        while(i<256 && BHT[i][13:2]!=PC[11:0] && BHT[i][13:2] != 0)
        begin
            i = i+1;
        end
        if(BHT[i][13:2] == 0 || BHT[i][1:0] == 2'b01 || BHT[i][1:0] == 2'b00)
            predict_jmp = 0;
        else 
            predict_jmp = 1;
        end
        end
    end
    always @(*)
    begin
        find = 1;
        while(find<256 && BHT[find][13:2] != add_PC[11:0] && BHT[find][13:2] != 0)
        begin
            find = find+1;
        end
    end
    always @(negedge clk)
    begin
         if (we == 1'b1)
        begin
            if(BHT[find][13:2] == 0)
            begin
                BHT[find][13:2] <= add_PC[11:0];
            end
            if(BHT[find][1:0] == 2'b00)
            begin 
                if(jmp_or_not == 0)
                    BHT[find][1:0] <= 2'b00;
                else                        
                    BHT[find][1:0] <= 2'b01;
            end
            else if(BHT[find][1:0] == 2'b01)
            begin
                if(jmp_or_not == 0)
                    BHT[find][1:0] <= 2'b00;
                else
                    BHT[find][1:0] <= 2'b11;
            end
            else if(BHT[find][1:0] == 2'b11)
            begin
                if(jmp_or_not == 0)
                    BHT[find][1:0] <= 2'b10;
                else
                    BHT[find][1:0] <= 2'b11;
            end
            else if(BHT[find][1:0] == 2'b10)
            begin
                if(jmp_or_not == 0)
                    BHT[find][1:0] <= 2'b00;
                else
                    BHT[find][1:0] <= 2'b11;
            end
        end
    end
endmodule


module BTBtable (
    input rst,
    input clk,
    input [63:0] PC,
    input [63:0] add_ID_PC,
    input [63:0] add_EX_PC,
    input [31:0] ID_IR,
    input [31:0] EX_IR,
    input ID_jmp_or_not,
    input EX_jmp_or_not,
    input ID_we,
    input EX_we,
    input [63:0] add_ID_jmp_PC,
    input [63:0] add_EX_jmp_PC,
    output reg if_foundBTB,                       //在BTB中是否查找到了跳转地址，若查找到了为1，否则为0
    output reg [63:0] jmp_PC
    );
    integer i;
    integer ID_find;
    integer EX_find;
    reg [127:0] BTB [1:256];
    initial 
    begin
        for(i = 1; i < 256; i = i + 1) BTB[i] <= 128'b0;
        if_foundBTB <= 1'b0;
        jmp_PC <= 64'b0;
    end
    always @(*)
    begin
        i = 1;
        while(i<256 && BTB[i][127:64]!=PC && BTB[i][127:64] != 0)
        begin
            i = i+1;
        end
        if(BTB[i][127:64] == 0)
        begin
            if_foundBTB = 1'b0;
            jmp_PC = 64'b0;
        end
        else
            if_foundBTB = 1'b1;
            jmp_PC = BTB[i][63:0];
    end
    always @(*)
    begin
        ID_find = 1;
        while(ID_find<256 && BTB[ID_find][127:64]!=add_ID_PC && BTB[ID_find][127:64] != 0)
        begin
            ID_find = ID_find+1;
        end
        EX_find = 1;
        while(EX_find<256 && BTB[EX_find][127:64]!=add_EX_PC && BTB[EX_find][127:64] != 0)
        begin
            EX_find = EX_find+1;
        end
    end
    always @(negedge clk)
    begin
        begin
            if (EX_we == 1'b1 && EX_jmp_or_not == 1'b1 || EX_IR[6:0] == 7'b1101111)
            begin
                if(BTB[EX_find][127:64] == 0)
                begin
                    BTB[EX_find][127:64] <= add_EX_PC;
                    BTB[EX_find][63:0] <= add_EX_jmp_PC;
                    
                end
            end
            if (ID_we == 1'b1 && ID_jmp_or_not == 1'b1 || ID_IR[6:0] == 7'b1101111)
            begin
                if(BTB[ID_find][127:64] == 0)
                begin
                    BTB[ID_find][127:64] <= add_ID_PC;
                    BTB[ID_find][63:0] <= add_ID_jmp_PC;
                end
            end
        end
    end

endmodule





module MUX_jmp (
    input [63:0] PC_temp,
    input predict_jmp,
    input [63:0] predict_jmp_PC,
    output reg [63:0] PC_final
    );
    always @(*)
    begin
        if(predict_jmp == 1)
            PC_final = predict_jmp_PC;
        else
            PC_final = PC_temp;
    end
endmodule



module MUX_pc (
    input [63:0] T0,T1,T3,
    input s,
    input [31:0] ID_IR,
    input ID_jmp_or_not,
    input ID_predict_jmp,
    output reg [63:0] o
    );
    always @(*) 
    begin
        if((ID_jmp_or_not == 1 || ID_IR[6:0] == 7'b1101111) && ID_predict_jmp == 1)
        begin
            o = T0;
        end
        else if(ID_jmp_or_not == 0 && ID_predict_jmp == 1 && ID_IR[6:0] != 7'b1101111)
            o = T3;
        else
            o = s? T1 : T0;
    end
endmodule