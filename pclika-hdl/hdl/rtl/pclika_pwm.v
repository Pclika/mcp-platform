/**
 * pclika_pwm.v — Multi-channel PWM Generator
 *
 * Generates N_CHANNELS independent PWM outputs.
 * Duty cycle per channel: 0 = always low, PERIOD-1 = always high.
 * Duty values are registered (updated at period boundary) to avoid glitches.
 *
 * Typical use (servo @ 50 Hz, 12 MHz clock):
 *   PERIOD = 12_000_000 / 50 = 240_000
 *   CNT_WIDTH = 18 (2^18 = 262144 > 240000)
 *   1 ms pulse  = 12000 cycles  → 0°
 *   1.5ms pulse = 18000 cycles  → 90°
 *   2 ms pulse  = 24000 cycles  → 180°
 *
 * Parameters:
 *   CLK_FREQ   — system clock in Hz
 *   PWM_FREQ   — PWM output frequency in Hz
 *   N_CHANNELS — number of independent PWM channels
 *   CNT_WIDTH  — counter bit width (must satisfy 2^CNT_WIDTH > CLK_FREQ/PWM_FREQ)
 *
 * Seal: PCK-MMXXVI-9198580D
 */

`default_nettype none
`timescale 1ns / 1ps

module pclika_pwm #(
    parameter CLK_FREQ   = 12_000_000,
    parameter PWM_FREQ   = 50,
    parameter N_CHANNELS = 4,
    parameter CNT_WIDTH  = 18
) (
    input  wire                          clk,
    input  wire                          rst_n,
    // Duty cycle per channel (raw cycle count, 0 to PERIOD-1)
    input  wire [N_CHANNELS*CNT_WIDTH-1:0] duty,
    output reg  [N_CHANNELS-1:0]         pwm_out
);

    localparam PERIOD = CLK_FREQ / PWM_FREQ;

    // ── Main counter ──────────────────────────────────────────────────────
    reg [CNT_WIDTH-1:0] cnt_r = {CNT_WIDTH{1'b0}};
    wire period_end = (cnt_r >= PERIOD[CNT_WIDTH-1:0] - 1'b1);

    always @(posedge clk) begin
        if (!rst_n)
            cnt_r <= {CNT_WIDTH{1'b0}};
        else if (period_end)
            cnt_r <= {CNT_WIDTH{1'b0}};
        else
            cnt_r <= cnt_r + 1'b1;
    end

    // ── Per-channel output ────────────────────────────────────────────────
    // Latch duty at period boundary to prevent mid-period glitches
    genvar i;
    generate
        for (i = 0; i < N_CHANNELS; i = i + 1) begin : ch
            reg [CNT_WIDTH-1:0] duty_latch_r;
            wire [CNT_WIDTH-1:0] duty_i = duty[i*CNT_WIDTH +: CNT_WIDTH];

            always @(posedge clk) begin
                if (!rst_n) begin
                    duty_latch_r <= {CNT_WIDTH{1'b0}};
                    pwm_out[i]   <= 1'b0;
                end else begin
                    if (period_end)
                        duty_latch_r <= duty_i;  // update at period boundary
                    pwm_out[i] <= (cnt_r < duty_latch_r);
                end
            end
        end
    endgenerate

endmodule

`default_nettype wire
