/**
 * pwm_gen_top.v — PWM Generator Example
 *
 * 4-channel PWM at 50 Hz (servo-compatible).
 * Duty per channel is controlled via PMOD GPIO inputs:
 *   btn_up  — increases CH0 duty by 1000 cycles per press
 *   btn_dn  — decreases CH0 duty by 1000 cycles per press
 *
 * Channel defaults (servo angles):
 *   CH0: 18000 cycles → 90° (1.5 ms)
 *   CH1: 12000 cycles → 0°  (1.0 ms)
 *   CH2: 24000 cycles → 180° (2.0 ms)
 *   CH3: 15000 cycles → ~45° (1.25 ms)
 *
 * Output: pwm_out[3:0] on PMOD1A[3:0]
 *
 * iCE40UP5K / iCEBreaker
 * Seal: PCK-MMXXVI-9198580D
 */

`default_nettype none
`timescale 1ns / 1ps

module pwm_gen_top #(
    parameter CLK_FREQ   = 12_000_000,
    parameter PWM_FREQ   = 50,
    parameter N_CHANNELS = 4,
    parameter CNT_WIDTH  = 18
) (
    input  wire clk,
    input  wire btn_up,    // active low
    input  wire btn_dn,    // active low
    output wire [N_CHANNELS-1:0] pwm_out,
    output wire led_r,
    output wire led_g,
    output wire led_b
);

    // ── Button debounce ───────────────────────────────────────────────────
    localparam DB_LEN = CLK_FREQ / 100;   // 10 ms debounce
    localparam DB_W   = $clog2(DB_LEN + 1);

    reg [DB_W-1:0] db_up_cnt_r = {DB_W{1'b0}};
    reg [DB_W-1:0] db_dn_cnt_r = {DB_W{1'b0}};
    reg            btn_up_sync_r = 1'b1, btn_dn_sync_r = 1'b1;
    reg            btn_up_db_r  = 1'b1, btn_dn_db_r   = 1'b1;
    reg            btn_up_prev_r = 1'b1, btn_dn_prev_r = 1'b1;
    wire           up_pressed = btn_up_prev_r & ~btn_up_db_r;   // falling edge
    wire           dn_pressed = btn_dn_prev_r & ~btn_dn_db_r;

    // 2-FF sync
    reg btn_up_ff1_r, btn_up_ff2_r, btn_dn_ff1_r, btn_dn_ff2_r;
    always @(posedge clk) begin
        btn_up_ff1_r <= btn_up;
        btn_up_ff2_r <= btn_up_ff1_r;
        btn_dn_ff1_r <= btn_dn;
        btn_dn_ff2_r <= btn_dn_ff1_r;
    end

    always @(posedge clk) begin
        btn_up_prev_r <= btn_up_db_r;
        btn_dn_prev_r <= btn_dn_db_r;

        if (btn_up_ff2_r !== btn_up_sync_r) begin
            db_up_cnt_r  <= {DB_W{1'b0}};
            btn_up_sync_r <= btn_up_ff2_r;
        end else if (db_up_cnt_r < DB_LEN[DB_W-1:0]) begin
            db_up_cnt_r <= db_up_cnt_r + 1'b1;
        end else begin
            btn_up_db_r <= btn_up_sync_r;
        end

        if (btn_dn_ff2_r !== btn_dn_sync_r) begin
            db_dn_cnt_r  <= {DB_W{1'b0}};
            btn_dn_sync_r <= btn_dn_ff2_r;
        end else if (db_dn_cnt_r < DB_LEN[DB_W-1:0]) begin
            db_dn_cnt_r <= db_dn_cnt_r + 1'b1;
        end else begin
            btn_dn_db_r <= btn_dn_sync_r;
        end
    end

    // ── Duty registers ────────────────────────────────────────────────────
    localparam DUTY_MIN  = 18'd12000;   // 1 ms  → 0°
    localparam DUTY_MAX  = 18'd24000;   // 2 ms  → 180°
    localparam DUTY_STEP = 18'd1000;    // ~15°

    reg [CNT_WIDTH-1:0] duty_ch0_r = 18'd18000;
    reg [CNT_WIDTH-1:0] duty_ch1_r = 18'd12000;
    reg [CNT_WIDTH-1:0] duty_ch2_r = 18'd24000;
    reg [CNT_WIDTH-1:0] duty_ch3_r = 18'd15000;

    always @(posedge clk) begin
        if (up_pressed && duty_ch0_r + DUTY_STEP <= DUTY_MAX)
            duty_ch0_r <= duty_ch0_r + DUTY_STEP;
        if (dn_pressed && duty_ch0_r >= DUTY_MIN + DUTY_STEP)
            duty_ch0_r <= duty_ch0_r - DUTY_STEP;
    end

    // ── PWM core ──────────────────────────────────────────────────────────
    wire [N_CHANNELS*CNT_WIDTH-1:0] duty_bus;
    assign duty_bus = {duty_ch3_r, duty_ch2_r, duty_ch1_r, duty_ch0_r};

    pclika_pwm #(
        .CLK_FREQ  (CLK_FREQ),
        .PWM_FREQ  (PWM_FREQ),
        .N_CHANNELS(N_CHANNELS),
        .CNT_WIDTH (CNT_WIDTH)
    ) u_pwm (
        .clk    (clk),
        .rst_n  (1'b1),
        .duty   (duty_bus),
        .pwm_out(pwm_out)
    );

    // ── LED: show CH0 position ────────────────────────────────────────────
    // Red   = at minimum (0°)
    // Green = at maximum (180°)
    // Blue  = midrange
    assign led_r = ~(duty_ch0_r <= DUTY_MIN + DUTY_STEP);
    assign led_g = ~(duty_ch0_r >= DUTY_MAX - DUTY_STEP);
    assign led_b = ~(duty_ch0_r > DUTY_MIN + DUTY_STEP &&
                     duty_ch0_r < DUTY_MAX - DUTY_STEP);

endmodule

`default_nettype wire
