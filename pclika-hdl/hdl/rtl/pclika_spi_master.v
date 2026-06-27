/**
 * pclika_spi_master.v — SPI Master (Mode 0: CPOL=0, CPHA=0)
 *
 * Full-duplex 8-bit SPI master.
 * SCK idles low. Data sampled on rising SCK edge (CPHA=0).
 * MSB transmitted first.
 *
 * Usage:
 *   1. Assert cs_n low (externally or via cs_auto parameter)
 *   2. Load tx_data, pulse tx_valid for 1 cycle when busy=0
 *   3. Wait for rx_valid to pulse — rx_data contains received byte
 *   4. Repeat for next byte, deassert cs_n when done
 *
 * Parameters:
 *   CLK_FREQ  — system clock in Hz
 *   SPI_FREQ  — SPI SCK frequency in Hz (CLK_FREQ / (2 * HALF_DIV))
 *   DATA_BITS — bits per transfer (default 8)
 *   CS_AUTO   — if 1, drive cs_n automatically per transfer
 *
 * Seal: PCK-MMXXVI-9198580D
 */

`default_nettype none
`timescale 1ns / 1ps

module pclika_spi_master #(
    parameter CLK_FREQ  = 12_000_000,
    parameter SPI_FREQ  = 1_000_000,
    parameter DATA_BITS = 8,
    parameter CS_AUTO   = 1
) (
    input  wire              clk,
    input  wire              rst_n,
    // Byte interface
    input  wire [DATA_BITS-1:0] tx_data,
    input  wire              tx_valid,
    output reg  [DATA_BITS-1:0] rx_data,
    output reg               rx_valid,
    output wire              busy,
    // SPI pins
    output reg               sck,
    output reg               mosi,
    input  wire              miso,
    output reg               cs_n
);

    localparam HALF_DIV  = CLK_FREQ / (SPI_FREQ * 2);
    localparam CNT_WIDTH = $clog2(HALF_DIV + 1);

    localparam S_IDLE = 2'd0;
    localparam S_SCK0 = 2'd1;   // SCK low phase
    localparam S_SCK1 = 2'd2;   // SCK high phase (sample MISO)
    localparam S_DONE = 2'd3;

    reg [1:0]             state_r    = S_IDLE;
    reg [CNT_WIDTH-1:0]   cnt_r      = {CNT_WIDTH{1'b0}};
    reg [$clog2(DATA_BITS):0] bit_r  = {($clog2(DATA_BITS)+1){1'b0}};
    reg [DATA_BITS-1:0]   tx_r       = {DATA_BITS{1'b0}};
    reg [DATA_BITS-1:0]   rx_r       = {DATA_BITS{1'b0}};

    assign busy = (state_r != S_IDLE);

    always @(posedge clk) begin
        rx_valid <= 1'b0;

        if (!rst_n) begin
            state_r <= S_IDLE;
            sck     <= 1'b0;
            mosi    <= 1'b1;
            cs_n    <= 1'b1;
            cnt_r   <= {CNT_WIDTH{1'b0}};
        end else begin
            case (state_r)

                S_IDLE: begin
                    sck  <= 1'b0;
                    mosi <= 1'b1;
                    if (CS_AUTO) cs_n <= 1'b1;
                    if (tx_valid) begin
                        tx_r    <= tx_data;
                        bit_r   <= DATA_BITS[($clog2(DATA_BITS)):0] - 1'b1;
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                        state_r <= S_SCK0;
                        if (CS_AUTO) cs_n <= 1'b0;
                    end
                end

                S_SCK0: begin
                    // SCK low — drive MOSI
                    sck  <= 1'b0;
                    mosi <= tx_r[DATA_BITS-1];  // MSB first
                    if (cnt_r == HALF_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                        state_r <= S_SCK1;
                    end else begin
                        cnt_r <= cnt_r + 1'b1;
                    end
                end

                S_SCK1: begin
                    // SCK high — sample MISO
                    sck <= 1'b1;
                    if (cnt_r == HALF_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r <= {CNT_WIDTH{1'b0}};
                        rx_r  <= {rx_r[DATA_BITS-2:0], miso};  // MSB first
                        tx_r  <= {tx_r[DATA_BITS-2:0], 1'b0};  // shift out
                        if (bit_r == {($clog2(DATA_BITS)+1){1'b0}}) begin
                            state_r <= S_DONE;
                        end else begin
                            bit_r   <= bit_r - 1'b1;
                            state_r <= S_SCK0;
                        end
                    end else begin
                        cnt_r <= cnt_r + 1'b1;
                    end
                end

                S_DONE: begin
                    sck      <= 1'b0;
                    rx_data  <= rx_r;
                    rx_valid <= 1'b1;
                    state_r  <= S_IDLE;
                end

            endcase
        end
    end

endmodule

`default_nettype wire
