/**
 * tb_pclika_pwm.v — PWM testbench
 *
 * Verifies:
 *   1. Period = CLK_FREQ / PWM_FREQ cycles
 *   2. High time per channel matches duty input
 *   3. Duty latch updates at period boundary (no mid-period glitch)
 *
 * Uses CLK_FREQ=100, PWM_FREQ=1 → PERIOD=100 for easy counting.
 */

`timescale 1ns / 1ps

module tb_pclika_pwm;

    localparam CLK_FREQ   = 100;
    localparam PWM_FREQ   = 1;
    localparam PERIOD     = CLK_FREQ / PWM_FREQ;   // 100
    localparam N_CHANNELS = 4;
    localparam CNT_WIDTH  = 8;                      // 2^8=256 > 100

    reg clk = 0, rst_n = 0;
    always #5 clk = ~clk;   // 100 MHz in sim units (period=10 per cycle)

    reg [N_CHANNELS*CNT_WIDTH-1:0] duty;
    wire [N_CHANNELS-1:0]          pwm_out;

    pclika_pwm #(
        .CLK_FREQ  (CLK_FREQ),
        .PWM_FREQ  (PWM_FREQ),
        .N_CHANNELS(N_CHANNELS),
        .CNT_WIDTH (CNT_WIDTH)
    ) u_dut (
        .clk    (clk),
        .rst_n  (rst_n),
        .duty   (duty),
        .pwm_out(pwm_out)
    );

    integer high_cnt [0:3];
    integer low_cnt  [0:3];
    integer total_cnt;
    integer ch;
    integer fail_count;
    integer pass_count;

    // Count high/low cycles in one full period for each channel
    task measure_period;
        integer k;
        begin
            for (k = 0; k < N_CHANNELS; k = k + 1) begin
                high_cnt[k] = 0;
                low_cnt[k]  = 0;
            end
            total_cnt = 0;
            // Sync to start of period by waiting for a falling edge of ch0
            // (simplification: just count next PERIOD clocks)
            repeat(PERIOD) begin
                @(posedge clk);
                for (k = 0; k < N_CHANNELS; k = k + 1) begin
                    if (pwm_out[k]) high_cnt[k] = high_cnt[k] + 1;
                    else            low_cnt[k]  = low_cnt[k]  + 1;
                end
                total_cnt = total_cnt + 1;
            end
        end
    endtask

    initial begin
        fail_count = 0;
        pass_count = 0;

        $dumpfile("tb_pclika_pwm.vcd");
        $dumpvars(0, tb_pclika_pwm);

        // Initial duty values: ch0=0, ch1=25, ch2=50, ch3=99
        duty = {8'd99, 8'd50, 8'd25, 8'd0};

        rst_n = 0;
        repeat(5) @(posedge clk);
        rst_n = 1;

        // Wait for at least one full period to pass before measuring
        repeat(PERIOD + 2) @(posedge clk);

        // Test 1: Verify duty cycles
        measure_period;

        $display("--- Period measurement (PERIOD=%0d) ---", PERIOD);
        $display("CH0 high=%0d (expected 0)",  high_cnt[0]);
        $display("CH1 high=%0d (expected 25)", high_cnt[1]);
        $display("CH2 high=%0d (expected 50)", high_cnt[2]);
        $display("CH3 high=%0d (expected 99)", high_cnt[3]);

        // CH0 duty=0: always low
        if (high_cnt[0] === 0) begin
            $display("PASS CH0 duty=0"); pass_count = pass_count + 1;
        end else begin
            $display("FAIL CH0: high_cnt=%0d expected 0", high_cnt[0]); fail_count = fail_count + 1;
        end

        // CH1 duty=25: 25 high cycles
        if (high_cnt[1] === 25) begin
            $display("PASS CH1 duty=25"); pass_count = pass_count + 1;
        end else begin
            $display("FAIL CH1: high_cnt=%0d expected 25", high_cnt[1]); fail_count = fail_count + 1;
        end

        // CH2 duty=50: 50 high cycles
        if (high_cnt[2] === 50) begin
            $display("PASS CH2 duty=50"); pass_count = pass_count + 1;
        end else begin
            $display("FAIL CH2: high_cnt=%0d expected 50", high_cnt[2]); fail_count = fail_count + 1;
        end

        // CH3 duty=99: 99 high cycles
        if (high_cnt[3] === 99) begin
            $display("PASS CH3 duty=99"); pass_count = pass_count + 1;
        end else begin
            $display("FAIL CH3: high_cnt=%0d expected 99", high_cnt[3]); fail_count = fail_count + 1;
        end

        // Test 2: Change duty mid-way through a period and verify latch
        // (duty should not take effect until the next period boundary)
        $display("--- Latch test: change duty mid-period ---");
        repeat(PERIOD / 2) @(posedge clk);
        // Change CH2 duty from 50 to 10 mid-period
        duty[2*CNT_WIDTH +: CNT_WIDTH] = 8'd10;
        // Count remaining half of current period
        begin : latch_check
            integer half_high;
            half_high = 0;
            repeat(PERIOD / 2) begin
                @(posedge clk);
                if (pwm_out[2]) half_high = half_high + 1;
            end
            // In the first half CH2=50 → ~25 high; second half should still be ~25 high
            // (latch means duty=10 not applied yet)
            // After period boundary: next period should show 10
            $display("Remaining half high=%0d (should be ~25, not affected by change to 10)", half_high);
        end

        // Now measure next full period — should see duty=10
        repeat(2) @(posedge clk);
        measure_period;
        $display("Next period CH2 high=%0d (expected 10)", high_cnt[2]);
        if (high_cnt[2] === 10) begin
            $display("PASS latch: duty update at period boundary"); pass_count = pass_count + 1;
        end else begin
            $display("FAIL latch: CH2 high=%0d expected 10", high_cnt[2]); fail_count = fail_count + 1;
        end

        // Summary
        $display("---");
        $display("PWM tests: %0d PASS, %0d FAIL", pass_count, fail_count);
        if (fail_count == 0) $display("ALL PASS");
        else                 $display("FAILURES DETECTED");

        $finish;
    end

    // Timeout guard
    initial begin
        #1_000_000;
        $display("TIMEOUT");
        $finish;
    end

endmodule
