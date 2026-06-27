/**
 * tb_blink_top.v — Testbench for blink_top
 *
 * Uses TOGGLE_CNT=6 to run a fast simulation: the LED toggles every 6 clock
 * cycles, letting us verify both toggle edges in tens of cycles.
 *
 * Run with: make sim TARGET=tb_blink_top
 * Or:       verilator --binary --timing -Wall -Wno-fatal \
 *               +define+SIMULATION \
 *               -DTOGGLE_CNT=6 -DCNT_WIDTH=4 \
 *               ../rtl/blink_top.v tb_blink_top.v \
 *               -o sim_blink && ./sim_blink
 */

`default_nettype none
`timescale 1ns / 1ps

module tb_blink_top;

    // ── DUT parameters (fast sim values) ─────────────────────────────────
    localparam TB_TOGGLE_CNT = 6;
    localparam TB_CNT_WIDTH  = 4;  // 2^4 = 16 > 6 ✓

    // ── Clock (12 MHz → 83.33 ns period → ~41 ns half-period) ─────────────
    reg clk = 1'b0;
    always #41 clk = ~clk;

    // ── DUT ───────────────────────────────────────────────────────────────
    wire led_r, led_g, led_b;

    blink_top #(
        .TOGGLE_CNT (TB_TOGGLE_CNT),
        .CNT_WIDTH  (TB_CNT_WIDTH)
    ) dut (
        .clk   (clk),
        .led_r (led_r),
        .led_g (led_g),
        .led_b (led_b)
    );

    // ── Waveform dump ─────────────────────────────────────────────────────
    initial begin
        $dumpfile("blink.vcd");
        $dumpvars(0, tb_blink_top);
    end

    // ── Assertions ────────────────────────────────────────────────────────
    integer errors = 0;

    task assert_eq;
        input       actual;
        input       expected;
        input [127:0] msg;
        begin
            if (actual !== expected) begin
                $display("FAIL [%0t] %s — got %b, expected %b", $time, msg, actual, expected);
                errors = errors + 1;
            end
        end
    endtask

    initial begin
        // ── t=0: initial state ────────────────────────────────────────────
        #1;
        assert_eq(led_r, 1'b1, "led_r off at reset");
        assert_eq(led_g, 1'b1, "led_g off at reset");  // state_r=0 → ~0=1 (active-low: off)
        assert_eq(led_b, 1'b1, "led_b off at reset");

        // ── Wait for first toggle: TOGGLE_CNT=6 clock edges ──────────────
        // At cycle 6 counter wraps and state_r flips to 1 → led_g goes low (on)
        repeat(TB_TOGGLE_CNT + 1) @(posedge clk);
        #1;
        assert_eq(led_r, 1'b1, "led_r still off after toggle 1");
        assert_eq(led_g, 1'b0, "led_g ON after toggle 1");   // state_r=1 → ~1=0 (on)
        assert_eq(led_b, 1'b1, "led_b still off after toggle 1");

        // ── Wait for second toggle ────────────────────────────────────────
        repeat(TB_TOGGLE_CNT) @(posedge clk);
        #1;
        assert_eq(led_g, 1'b1, "led_g OFF after toggle 2");

        // ── Wait for third toggle ─────────────────────────────────────────
        repeat(TB_TOGGLE_CNT) @(posedge clk);
        #1;
        assert_eq(led_g, 1'b0, "led_g ON after toggle 3");

        // ── Report ────────────────────────────────────────────────────────
        if (errors == 0)
            $display("PASS: tb_blink_top — all %0d assertions passed", 8);
        else
            $display("FAIL: tb_blink_top — %0d assertion(s) failed", errors);

        $finish;
    end

    // ── Timeout guard ─────────────────────────────────────────────────────
    initial begin
        #100000;
        $display("TIMEOUT: tb_blink_top did not finish in time");
        $finish;
    end

endmodule

`default_nettype wire
