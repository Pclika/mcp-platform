/**
 * pclika_uart_rx.v — UART Receiver (8N1)
 *
 * Receives serial data: 1 start bit, 8 data bits (LSB first), 1 stop bit.
 * Includes 2-FF CDC synchronizer on rx input.
 * Outputs data[7:0] with a 1-cycle data_valid pulse on completion.
 * framing_err pulses 1 cycle if stop bit is not high.
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

module pclika_uart_rx #(
    parameter CLK_FREQ  = 12_000_000,
    parameter BAUD_RATE = 115_200,
    parameter DATA_BITS = 8
) (
    input  wire              clk,
    input  wire              rst_n,
    input  wire              rx,
    output reg  [DATA_BITS-1:0] data,
    output reg               data_valid,
    output reg               framing_err
);

    localparam BAUD_DIV  = CLK_FREQ / BAUD_RATE;
    localparam HALF_DIV  = BAUD_DIV / 2;
    localparam CNT_WIDTH = $clog2(BAUD_DIV + 1);

    localparam S_IDLE  = 2'd0;
    localparam S_START = 2'd1;
    localparam S_DATA  = 2'd2;
    localparam S_STOP  = 2'd3;

    // ── CDC synchronizer ──────────────────────────────────────────────────
    reg rx_ff1_r, rx_ff2_r;
    always @(posedge clk) begin
        rx_ff1_r <= rx;
        rx_ff2_r <= rx_ff1_r;
    end
    wire rx_s = rx_ff2_r;

    // ── State machine ─────────────────────────────────────────────────────
    reg [1:0]            state_r    = S_IDLE;
    reg [CNT_WIDTH-1:0]  cnt_r      = {CNT_WIDTH{1'b0}};
    reg [$clog2(DATA_BITS):0] bit_r = {($clog2(DATA_BITS)+1){1'b0}};
    reg [DATA_BITS-1:0]  shift_r    = {DATA_BITS{1'b0}};

    always @(posedge clk) begin
        data_valid  <= 1'b0;
        framing_err <= 1'b0;

        if (!rst_n) begin
            state_r <= S_IDLE;
            cnt_r   <= {CNT_WIDTH{1'b0}};
            bit_r   <= {($clog2(DATA_BITS)+1){1'b0}};
        end else begin
            case (state_r)

                S_IDLE: begin
                    if (!rx_s) begin          // falling edge: start bit
                        state_r <= S_START;
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                    end
                end

                S_START: begin
                    // Wait half bit period, sample in centre of start bit
                    if (cnt_r == HALF_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r <= {CNT_WIDTH{1'b0}};
                        if (!rx_s) begin
                            state_r <= S_DATA;
                            bit_r   <= {($clog2(DATA_BITS)+1){1'b0}};
                        end else begin
                            state_r <= S_IDLE;  // glitch
                        end
                    end else begin
                        cnt_r <= cnt_r + 1'b1;
                    end
                end

                S_DATA: begin
                    if (cnt_r == BAUD_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                        shift_r <= {rx_s, shift_r[DATA_BITS-1:1]};  // LSB first
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
                    if (cnt_r == BAUD_DIV[CNT_WIDTH-1:0] - 1'b1) begin
                        cnt_r   <= {CNT_WIDTH{1'b0}};
                        state_r <= S_IDLE;
                        if (rx_s) begin
                            data       <= shift_r;
                            data_valid <= 1'b1;
                        end else begin
                            framing_err <= 1'b1;
                        end
                    end else begin
                        cnt_r <= cnt_r + 1'b1;
                    end
                end

            endcase
        end
    end

endmodule

`default_nettype wire
