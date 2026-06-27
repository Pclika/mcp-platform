/**
 * blink_top.v — Pclika HDL: Hello FPGA
 *
 * Blinks the RGB LED at BLINK_HZ on any iCE40UP5K board (iCEBreaker default).
 * RGB LED on iCEBreaker is active-low: 0 = on, 1 = off.
 *
 * Parameters:
 *   TOGGLE_CNT  — clock cycles between each LED toggle (default: 6_000_000 for 1 Hz at 12 MHz)
 *   CNT_WIDTH   — counter bit width, must satisfy 2^CNT_WIDTH > TOGGLE_CNT
 *
 * Seal: PCK-MMXXVI-9198580D
 */

`default_nettype none
`timescale 1ns / 1ps

module blink_top #(
    parameter TOGGLE_CNT = 6_000_000,  // cycles per half-period → 1 Hz at 12 MHz
    parameter CNT_WIDTH  = 24           // 2^24 = 16M > 6M ✓
) (
    input  wire clk,    // System clock (12 MHz on iCEBreaker)
    output wire led_r,  // RGB red   (active low)
    output wire led_g,  // RGB green (active low)
    output wire led_b   // RGB blue  (active low)
);

    // ── Clock divider counter ─────────────────────────────────────────────
    reg [CNT_WIDTH-1:0] cnt_r  = {CNT_WIDTH{1'b0}};
    reg                 state_r = 1'b0;  // 0 = LED off, 1 = LED on

    always @(posedge clk) begin
        if (cnt_r == TOGGLE_CNT[CNT_WIDTH-1:0] - 1'b1) begin
            cnt_r   <= {CNT_WIDTH{1'b0}};
            state_r <= ~state_r;
        end else begin
            cnt_r <= cnt_r + 1'b1;
        end
    end

    // ── LED output ────────────────────────────────────────────────────────
    // Blink green; red and blue stay off
    assign led_r = 1'b1;        // off
    assign led_g = ~state_r;    // blink (active low: 0 = on)
    assign led_b = 1'b1;        // off

endmodule

`default_nettype wire
