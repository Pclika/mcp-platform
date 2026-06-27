/**
 * uart_echo_top.v — UART Echo Example
 *
 * Receives bytes on uart_rx and immediately retransmits them on uart_tx.
 * LED blinks green on each received byte; red if framing error.
 *
 * Parameters:
 *   CLK_FREQ  — system clock in Hz  (default: 12 MHz iCEBreaker)
 *   BAUD_RATE — serial baud rate    (default: 115200)
 *
 * iCE40UP5K / iCEBreaker pinout: see constraints/ice40up5k.pcf
 *
 * Seal: PCK-MMXXVI-9198580D
 */

`default_nettype none
`timescale 1ns / 1ps

module uart_echo_top #(
    parameter CLK_FREQ  = 12_000_000,
    parameter BAUD_RATE = 115_200
) (
    input  wire clk,
    input  wire uart_rx,
    output wire uart_tx,
    output wire led_r,    // active low: lights on framing error
    output wire led_g,    // active low: blinks on received byte
    output wire led_b     // active low: idle heartbeat
);

    // ── UART RX ──────────────────────────────────────────────────────────
    wire [7:0] rx_data;
    wire       rx_valid;
    wire       rx_framing_err;

    pclika_uart_rx #(
        .CLK_FREQ (CLK_FREQ),
        .BAUD_RATE(BAUD_RATE)
    ) u_rx (
        .clk        (clk),
        .rst_n      (1'b1),
        .rx         (uart_rx),
        .data       (rx_data),
        .data_valid (rx_valid),
        .framing_err(rx_framing_err)
    );

    // ── Echo buffer (single-byte: fine for interactive use) ───────────────
    // If TX is busy when rx_valid arrives, the byte is held until TX is free.
    wire tx_busy;
    reg  [7:0] echo_r      = 8'h00;
    reg        echo_valid_r = 1'b0;

    always @(posedge clk) begin
        if (rx_valid && !tx_busy) begin
            echo_r      <= rx_data;
            echo_valid_r <= 1'b1;
        end else if (!tx_busy) begin
            // hold data but keep valid only for one cycle when TX not busy
            echo_valid_r <= rx_valid;
            if (rx_valid) echo_r <= rx_data;
        end
        if (echo_valid_r && !tx_busy)
            echo_valid_r <= 1'b0;
    end

    // ── UART TX ──────────────────────────────────────────────────────────
    pclika_uart_tx #(
        .CLK_FREQ (CLK_FREQ),
        .BAUD_RATE(BAUD_RATE)
    ) u_tx (
        .clk       (clk),
        .rst_n     (1'b1),
        .data      (echo_r),
        .data_valid(echo_valid_r),
        .tx        (uart_tx),
        .busy      (tx_busy)
    );

    // ── LED indicators ───────────────────────────────────────────────────
    // Green: 100ms pulse on each received byte
    localparam PULSE_LEN = CLK_FREQ / 10;   // 100 ms
    localparam P_WIDTH   = $clog2(PULSE_LEN + 1);

    reg [P_WIDTH-1:0] pulse_cnt_r  = {P_WIDTH{1'b0}};
    reg               led_g_r      = 1'b0;
    reg               err_latch_r  = 1'b0;

    always @(posedge clk) begin
        if (rx_valid) begin
            led_g_r     <= 1'b1;
            pulse_cnt_r <= {P_WIDTH{1'b0}};
        end else if (led_g_r) begin
            if (pulse_cnt_r >= PULSE_LEN[P_WIDTH-1:0] - 1'b1) begin
                led_g_r     <= 1'b0;
                pulse_cnt_r <= {P_WIDTH{1'b0}};
            end else begin
                pulse_cnt_r <= pulse_cnt_r + 1'b1;
            end
        end
        if (rx_framing_err) err_latch_r <= 1'b1;
    end

    // Blue: slow heartbeat ~1 Hz
    localparam HB_LEN = CLK_FREQ / 2;
    localparam H_WIDTH = $clog2(HB_LEN + 1);
    reg [H_WIDTH-1:0] hb_cnt_r = {H_WIDTH{1'b0}};
    reg               hb_r     = 1'b0;
    always @(posedge clk) begin
        if (hb_cnt_r >= HB_LEN[H_WIDTH-1:0] - 1'b1) begin
            hb_cnt_r <= {H_WIDTH{1'b0}};
            hb_r     <= ~hb_r;
        end else begin
            hb_cnt_r <= hb_cnt_r + 1'b1;
        end
    end

    // Active-low LED outputs
    assign led_r = ~err_latch_r;   // red stays on after framing error
    assign led_g = ~led_g_r;
    assign led_b = ~hb_r;

endmodule

`default_nettype wire
