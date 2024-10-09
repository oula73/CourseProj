`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2022/12/03 10:21:51
// Design Name: 
// Module Name: myRam
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


module myRam(
    input clk,
    input rst,
    input we,
    input [1:0]  LBinRam,
    input [63:0] write_data,
    input [10:0] address,
    output [63:0] read_data
    );
    reg [63:0] ram [0:2047];
    integer i;
    wire [10:0] index, offset;
    reg [63:0] temp1, temp2;
    
    assign index = (address / 8)*8;
    assign offset = (address % 8)*8;

    always @(negedge rst) begin
        for(i = 0; i < 2047; i = i + 1)
            ram[i] = 0;
    end
    always @(posedge clk) begin
        temp1 <= 0;temp2 <= 0;
        if (we == 1) 
            begin 
                if(LBinRam == 2'b00)      begin ram[index] <= write_data; end
                else if(LBinRam == 2'b10) begin 
                    temp1 <= 0;
                    temp2 <= (-1)^(32'hffffffff<<offset);
                    temp1 <= write_data[31:0]<< offset;
                    ram[index] <= (ram[index]&temp2)|temp1;
                end
                else if(LBinRam == 2'b01) begin
                    temp1 <= 0;
                    temp2 <= (-1)^(8'hff<<offset);
                    temp1 <= write_data[7:0]<< offset;
                    ram[index] <= (ram[index]&temp2)|temp1;
                end
                else begin ram[index] <= ram[index]; end
            end
    end

    assign read_data = ram[index];

endmodule


module MMU(
    input [63:0] logical_ram_addr_in,
    input [63:0] logical_pc_addr_in,
    output reg  [63:0] physical_ram_addr_out,
    output reg  [63:0] physical_pc_addr_out
    );
    
endmodule
