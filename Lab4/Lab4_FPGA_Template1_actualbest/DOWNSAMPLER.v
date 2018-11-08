module DOWNSAMPLER
(
	data_val,
	pixel_in,
	pixel_out,
	W_EN,
	clk
);

input data_val;
input clk;
input [15:0] pixel_in;
output [7:0] pixel_out;
output W_EN;

reg w_en;
reg [2:0] green;
reg [2:0] red;
reg [1:0] blue;

//assign pixel_out = {red, blue, green};
assign pixel_out[7:5] = red;
assign pixel_out[4:2] = green;
assign pixel_out[1:0] = blue;
assign W_EN = w_en;

always @ (posedge clk) begin
	if (data_val) begin
		red <= pixel_in[15:13];
		blue <= pixel_in[4:3];
		green <= pixel_in[10:8];
		w_en <= 1;
	end
	else begin
		red <= 3'd0;
		blue <= 2'd0;
		green <= 3'd0;
		w_en <= 0;
	end
end

endmodule