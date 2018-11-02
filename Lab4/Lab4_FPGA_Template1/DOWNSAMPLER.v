module DOWNSAMPLER
(
	pixel_in,
	pixel_out,
	W_EN,
	PCLK, 
	HREF, 
	VSYNC
);

input PCLK;
input HREF;
input VSYNC;
input [7:0] pixel_in;
output [7:0] pixel_out;
output reg W_EN;

reg byteNum = 0;      
input data_val;

always @ (posedge PCLK) begin
	if(byteNum == 0 && HREF) begin
		pixel_out[7:5] <= pixel_in[7:5];
		pixel_out[4:3] <= pixel_in[2:1];
		pixel_out[3:0] <= 3'd0;
		byteNum <= 1;
		W_EN <= 1'd0;
	end else if(byteNum == 1 && HREF) begin
		pixel_out[7:5] <= pixel_out[7:5];
		pixel_out[4:3] <= pixel_out[4:3];
		pixel_out[3:0] <= pixel_in[4:2];
		byteNum <= 0;
		W_EN <= 1'd1;
	end
end

endmodule