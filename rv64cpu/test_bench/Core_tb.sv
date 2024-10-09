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


module Core_tb
    #(parameter T = 40)();
    // input
    logic        clk;
    logic        aresetn;
    logic        step;
    logic        debug_mode;
    logic [4:0]  debug_reg_addr; // register address
    logic [63:0] data_in;
    logic [63:0] chip_debug_in;
    // output
    logic [63:0] address;
    logic [63:0] data_out;
    logic [31:0] chip_debug_out0;
    logic [63:0] chip_debug_out1;
    logic [31:0] chip_debug_out2;
    logic [31:0] chip_debug_out3;
    logic [63:0]  ra ;
        logic [63:0]  sp ;
        logic [63:0]  gp ;
        logic [63:0]  tp ;
        logic [63:0]  t0 ;
        logic [63:0]  t1 ;
        logic [63:0]  t2 ;
        logic [63:0]  s0 ;
        logic [63:0]  s1 ;
        logic [63:0]  a0 ;
        logic [63:0]  a1 ;
        logic [63:0]  a2 ;
        logic [63:0]  a3 ;
        logic [63:0]  a4 ;
        logic [63:0]  a5 ;
        logic [63:0]  a6 ;
        logic [63:0]  a7 ;
        logic [63:0]  s2 ;
        logic [63:0]  s3 ;
        logic [63:0]  s4 ;
        logic [63:0]  s5 ;
        logic [63:0]  s6 ;
        logic [63:0]  s7 ;
        logic [63:0]  s8 ;
        logic [63:0]  s9 ;
        logic [63:0]  s10;
        logic [63:0]  s11;
        logic [63:0]  t3 ;
        logic [63:0]  t4 ;
        logic [63:0]  t5 ;
        logic [63:0]  t6 ;
    // local
    logic [31:0] inst;
    logic [63:0]pc_out, addr_out;

    Core uut(
        .clk(clk),
        .aresetn(aresetn),
        .step(step),
        .debug_mode(debug_mode),
         .debug_reg_addr(debug_reg_addr), // register address
        .address(address),
        .data_out(data_out),
        .data_in(data_in),
        .chip_debug_in(chip_debug_in),
        .chip_debug_out0(chip_debug_out0),
        .chip_debug_out1(chip_debug_out1),
        .chip_debug_out2(chip_debug_out2),
        .chip_debug_out3(chip_debug_out3),
        .ra(ra),   
                .sp(sp),   
                .gp(gp),   
                .tp(tp),   
                .t0(t0),   
                .t1(t1),   
                .t2(t2),   
                .s0(s0),   
                .s1(s1),   
                .a0(a0),   
                .a1(a1),   
                .a2(a2),   
                .a3(a3),   
                .a4(a4),   
                .a5(a5),   
                .a6(a6),   
                .a7(a7),   
                .s2(s2),   
                .s3(s3),   
                .s4(s4),   
                .s5(s5),   
                .s6(s6),   
                .s7(s7),   
                .s8(s8),   
                .s9(s9),   
                .s10(s10), 
                .s11(s11), 
                .t3(t3),   
                .t4(t4),   
                .t5(t5),   
                .t6(t6)    
    );

    assign pc_out = chip_debug_out0;
    assign addr_out = chip_debug_out1;
    assign inst = chip_debug_out2;
    

    integer i;
    initial begin
        aresetn = 0;
        clk = 1;
        step = 0;
        debug_mode = 1;
        #100;
        
        fork
            forever #(T/2) clk <= ~clk;
            #(2*T) aresetn = 1;
        join
    end
endmodule
