`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/05/28 20:34:18
// Design Name: 
// Module Name: Core_tb
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

module Core(
    input  wire        clk,
    input  wire        aresetn,
    input  wire        step,
    input  wire        debug_mode,
    input  wire [4:0]  debug_reg_addr, // register address

    output wire [63:0] address,
    output wire [63:0] data_out,
    
    input  wire [63:0] data_in,
    
    input  wire [63:0] chip_debug_in,
    output wire [63:0] chip_debug_out0,
    output wire [63:0] chip_debug_out1,
    output wire [31:0] chip_debug_out2,
    output wire [31:0] chip_debug_out3,
    
    output  wire    [31:0]  ra ,
    output  wire    [31:0]  sp ,
    output  wire    [31:0]  gp ,
    output  wire    [31:0]  tp ,
    output  wire    [31:0]  t0 ,
    output  wire    [31:0]  t1 ,
    output  wire    [31:0]  t2 ,
    output  wire    [31:0]  s0 ,
    output  wire    [31:0]  s1 ,
    output  wire    [31:0]  a0 ,
    output  wire    [31:0]  a1 ,
    output  wire    [31:0]  a2 ,
    output  wire    [31:0]  a3 ,
    output  wire    [31:0]  a4 ,
    output  wire    [31:0]  a5 ,
    output  wire    [31:0]  a6 ,
    output  wire    [31:0]  a7 ,
    output  wire    [31:0]  s2 ,
    output  wire    [31:0]  s3 ,
    output  wire    [31:0]  s4 ,
    output  wire    [31:0]  s5 ,
    output  wire    [31:0]  s6 ,
    output  wire    [31:0]  s7 ,
    output  wire    [31:0]  s8 ,
    output  wire    [31:0]  s9 ,
    output  wire    [31:0]  s10,
    output  wire    [31:0]  s11,
    output  wire    [31:0]  t3 ,
    output  wire    [31:0]  t4 ,
    output  wire    [31:0]  t5 ,
    output  wire    [31:0]  t6 
);
    wire rst, mem_write, mem_clk, cpu_clk;
    wire [1:0]  LBinRam;
    wire [31:0] inst;
    wire [63:0] core_data_in, addr_out, core_data_out, pc_out;
    reg  [31:0] clk_div;
    wire [63:0] wbdata;
    assign  data_out = core_data_out;
    wire [63:0]  regst [1:31];
    assign rst = ~aresetn;
    SCPU cpu(
        .clk(cpu_clk),
        .rst(rst),
        .inst(inst),                 //instruction from instruction memory
        .data_in(core_data_in),      // data from data memory
        .addr_out(addr_out),         // data memory address
        .data_out(core_data_out),    // data to data memory
        .pc_out(pc_out),             // connect to instruction memory
        .mem_write(mem_write),
        .LBinRam(LBinRam),
        .ra (regst[1] ),                  // output wire [31 : 0] ra
        .sp (regst[2] ),                  // output wire [31 : 0] sp
        .gp (regst[3] ),                  // output wire [31 : 0] gp
        .tp (regst[4] ),                  // output wire [31 : 0] tp
        .t0 (regst[5] ),                  // output wire [31 : 0] t0
        .t1 (regst[6] ),                  // output wire [31 : 0] t1
        .t2 (regst[7] ),                  // output wire [31 : 0] t2
        .s0 (regst[8] ),                  // output wire [31 : 0] s0
        .s1 (regst[9] ),                  // output wire [31 : 0] s1
        .a0 (regst[10]),                  // output wire [31 : 0] a0
        .a1 (regst[11]),                  // output wire [31 : 0] a1
        .a2 (regst[12]),                  // output wire [31 : 0] a2
        .a3 (regst[13]),                  // output wire [31 : 0] a3
        .a4 (regst[14]),                  // output wire [31 : 0] a4
        .a5 (regst[15]),                  // output wire [31 : 0] a5
        .a6 (regst[16]),                  // output wire [31 : 0] a6
        .a7 (regst[17]),                  // output wire [31 : 0] a7
        .s2 (regst[18]),                  // output wire [31 : 0] s2
        .s3 (regst[19]),                  // output wire [31 : 0] s3
        .s4 (regst[20]),                  // output wire [31 : 0] s4
        .s5 (regst[21]),                  // output wire [31 : 0] s5
        .s6 (regst[22]),                  // output wire [31 : 0] s6
        .s7 (regst[23]),                  // output wire [31 : 0] s7
        .s8 (regst[24]),                  // output wire [31 : 0] s8
        .s9 (regst[25]),                  // output wire [31 : 0] s9
        .s10(regst[26]),                // output wire [31 : 0] s10
        .s11(regst[27]),                // output wire [31 : 0] s11
        .t3 (regst[28]),                  // output wire [31 : 0] t3
        .t4 (regst[29]),                  // output wire [31 : 0] t4
        .t5 (regst[30]),                  // output wire [31 : 0] t5
        .t6 (regst[31]) ,                 // output wire [31 : 0] t6
       .WB_data(wbdata)
    );
    
    always @(posedge clk) begin
        if(rst) clk_div <= 0;
        else clk_div <= clk_div + 1;
    end
    assign mem_clk = ~clk_div[0]; // 50mhz
    assign cpu_clk = debug_mode ? clk_div[0] : step;
    
    wire    [63:0]  pc_out_0;
    wire    [3:0]   tem;
    assign  tem = pc_out[3:0];
    assign  pc_out_0 = pc_out / 4;
   // assign  pc_out = pc_out_0 << 4; 
    // TODO: 连接Instruction Memory
//    Rom rom_unit (
//        .a(pc_out[12:2]),  // 地址输入
//        .spo(inst)     //  读数据输�?
//    );
    myRom rom_unit(
        .address(pc_out_0),
        .out(inst)
    );


    
     // TODO: 连接Data Memory
//   Ram ram_unit (
//        .clka(clk),  // 时钟
//        .wea(mem_write),   // 是否写数据使能信�?
//        .addra(addr_out), // 地址输入
//        .dina(core_data_out),  // 写数据输�?
//        .douta(core_data_in)  // 读数据输�?
//    );
   myRam ram_unit(
        .clk(clk),
        .rst(rst),
        .we(mem_write),
        .LBinRam(LBinRam),
        .address(addr_out),
        .write_data(core_data_out),
        .read_data(core_data_in)
    );
    assign  ra  =   regst[1] ;
    assign  sp  =   regst[2] ;
    assign  gp  =   regst[3] ;
    assign  tp  =   regst[4] ;
    assign  t0  =   regst[5] ;
    assign  t1  =   regst[6] ;
    assign  t2  =   regst[7] ;
    assign  s0  =   regst[8] ;
    assign  s1  =   regst[9] ;
    assign  a0  =   regst[10];
    assign  a1  =   regst[11];
    assign  a2  =   regst[12];
    assign  a3  =   regst[13];
    assign  a4  =   regst[14];
    assign  a5  =   regst[15];
    assign  a6  =   regst[16];
    assign  a7  =   regst[17];
    assign  s2  =   regst[18];
    assign  s3  =   regst[19];
    assign  s4  =   regst[20];
    assign  s5  =   regst[21];
    assign  s6  =   regst[22];
    assign  s7  =   regst[23];
    assign  s8  =   regst[24];
    assign  s9  =   regst[25];
    assign  s10 =   regst[26];
    assign  s11 =   regst[27];
    assign  t3  =   regst[28];
    assign  t4  =   regst[29];
    assign  t5  =   regst[30];
    assign  t6  =   regst[31];
     // TODO: 添加32个寄存器组的输出
    assign chip_debug_out0 = pc_out;        //pc
    assign chip_debug_out1 = addr_out;      //访存地址
    assign chip_debug_out2 = inst;      //内存内容
    assign chip_debug_out3 = gp;//regst[debug_reg_addr];  //寄存器内�?
  
endmodule