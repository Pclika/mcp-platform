/**
 * pclika_uart_tx.v — UART Transmitter (8N1)
 *
 * Transmits serial data: 1 start bit, 8 data bits (LSB first), 1 stop bit.
 * busy is high whenever a transmission is in progress.
 * Load data and assert data_valid for 1 cycle when busy=0.
 *
 * Parameters:
 *   CLK_FREQ   — system clock frequency in Hz (default 12 MHz)
 *   BAUD_RATE  — baud rate in bps (default 115200)
 *   DATA_BITS  — data bits per frame (default 8)
 *
 * Seal: PCK-MMXXVI-9198580D
 */

`default_nettype none
`timescale 1ns / 1ps

module pclika_uart_tx #(
    parameter CLK_FREQ  = 12_000_000,
    parameter BAUD_RATE = 115_200,
    parameter DATA_BITS = 8
) (
    input  wire              clk,
    input  wire              rst_n,
    input  wire [DATA_BITS-1:0] data,
    input  wire              data_valid,
    output reg               tx,
    output wire              busy
);

    localparam BAUD_DIV  = CLK_FREQ / BAUD_RATE;
    localparam CNT_WIDTH = $clog2(BAUD_DIV + 1);

    localparam S_IDLE  = 2'd0;
    localparam S_START = 2'd1;
    localparam S_DATA  = 2'd2;
    localparam S_STOP  = 2'd3;

    reg [1:0]            state_r = S_IDLE;
    reg [CNT_WIDTH-1:0]  cnt_r   = {CNT_WIDTH{1'b0}};
    reg [$clog2(DATA_BITS):0] bit_r = {($clog2(DATA_BITS)+1){1'b0}};
    reg [DATA_BITS-1:0]  shift_r = {DATA_BITS{1'b0}};

    assign busy = (state_r != S_IDLE);

    always @(posedge clk) begin
        if (!rst_n) begin
            state_r <= S_IDLE;
            tx      <= 1'b1;
            cnt_r   <= {CNT_WIDTH{1'b0}};
            bit_r   <= {($clog2(DATA_BITS)+1){1'b0}};
        end else begin
            case (state_r)

                S_IDLE: begin
                    tx <= 1'b1;
                    if (data_valid) begin
                        shift_r <= data;
                        state_r <= S_START;
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                    end
                end

                S_START: begin
                    tx <= 1'b0;   // start bit
                    if (cnt_r == BAUD_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                        state_r <= S_DATA;
                        bit_r   <= {($clog2(DATA_BITS)+1){1'b0}};
                    end else begin
                        cnt_r <= cnt_r + 1'b1;
                    end
                end

                S_DATA: begin
                    tx <= shift_r[0];  // LSB first
                    if (cnt_r == BAUD_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                        shift_r <= {1'b0, shift_r[DATA_BITS-1:1]};
                        if (bit_r == DATA_BITS - 1) begin
                            state_r <= S_STOP;
                        end else begin
                            bit_r <= bit_r + 1'b1;
                        end
                    end else begin
                        cnt_r <= cnt_r + 1'b1;
                    end
                end

                S_STOP: begin
                    tx <= 1'b1;   // stop bit
                    if (cnt_r == BAUD_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                        state_r <= S_IDLE;
                    end else begin
                        cnt_r <= cnt_r + 1'b1;
                    end
                end

            endcase
        end
    end

endmodule

`default_nettype wire
