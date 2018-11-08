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
input [7:0] pixel_in;
output [7:0] pixel_out;
output W_EN;

reg w_en;
reg [1:0] green;
reg [2:0] red;
reg [2:0] blue;

//assign pixel_out = {red, blue, green};
assign pixel_out[7:5] = red;
assign pixel_out[4:3] = green;
assign pixel_out[2:0] = blue;
assign W_EN = w_en;
reg val = 0;

always @ (posedge clk) begin
	if (!data_val) begin
		red <= pixel_in[7:5];
		blue <= blue;
		green <= pixel_in[2:1];
		w_en <= 0;
	end
	else begin
		red <= red;
		blue <= pixel_in[4:2];
		green <= green;
		w_en <= 1;
	end
end

endmodule