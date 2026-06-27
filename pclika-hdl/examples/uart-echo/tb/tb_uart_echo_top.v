/**
 * tb_uart_echo_top.v — Testbench for uart_echo_top
 *
 * Drives a UART stimulus byte-by-byte into uart_rx,
 * captures uart_tx output, and verifies round-trip loopback.
 *
 * Stimulus: "Hello\r\n" (7 bytes)
 *
 * Seal: PCK-MMXXVI-9198580D
 */

`timescale 1ns / 1ps

module tb_uart_echo_top;

    localparam CLK_FREQ  = 12_000_000;
    localparam BAUD_RATE = 1_200_000;  // 10 clocks per bit for fast sim
    localparam BIT_TICKS = CLK_FREQ / BAUD_RATE;  // 10

    reg clk = 0;
    always #41 clk = ~clk;   // ~12 MHz

    // TX stimulus wire driven by task
    reg uart_rx_stim = 1'b1;

    // Captured outputs
    wire uart_tx_cap;
    wire led_r, led_g, led_b;

    uart_echo_top #(
        .CLK_FREQ (CLK_FREQ),
        .BAUD_RATE(BAUD_RATE)
    ) u_dut (
        .clk     (clk),
        .uart_rx (uart_rx_stim),
        .uart_tx (uart_tx_cap),
        .led_r   (led_r),
        .led_g   (led_g),
        .led_b   (led_b)
    );

    // ── Task: send one byte over uart_rx ──────────────────────────────────
    task send_byte;
        input [7:0] b;
        integer j;
        begin
            // Start bit
            uart_rx_stim = 1'b0;
            repeat(BIT_TICKS) @(posedge clk);
            // Data bits LSB first
            for (j = 0; j < 8; j = j + 1) begin
                uart_rx_stim = b[j];
                repeat(BIT_TICKS) @(posedge clk);
            end
            // Stop bit
            uart_rx_stim = 1'b1;
            repeat(BIT_TICKS) @(posedge clk);
        end
    endtask

    // ── Task: capture one byte from uart_tx ───────────────────────────────
    task recv_byte;
        output [7:0] b;
        integer j;
        begin
            // Wait for start bit (falling edge)
            @(negedge uart_tx_cap);
            // Skip to center of start bit
            repeat(BIT_TICKS / 2) @(posedge clk);
            // Sample data bits
            for (j = 0; j < 8; j = j + 1) begin
                repeat(BIT_TICKS) @(posedge clk);
                b[j] = uart_tx_cap;
            end
            // Stop bit
            repeat(BIT_TICKS) @(posedge clk);
        end
    endtask

    // ── Stimulus ──────────────────────────────────────────────────────────
    reg  [7:0] test_bytes [0:6];
    reg  [7:0] received;
    integer    pass_count, fail_count, i;

    initial begin
        test_bytes[0] = "H";
        test_bytes[1] = "e";
        test_bytes[2] = "l";
        test_bytes[3] = "l";
        test_bytes[4] = "o";
        test_bytes[5] = "\r";
        test_bytes[6] = "\n";
        pass_count = 0;
        fail_count = 0;

        $dumpfile("tb_uart_echo_top.vcd");
        $dumpvars(0, tb_uart_echo_top);

        // Settle
        repeat(10) @(posedge clk);

        for (i = 0; i < 7; i = i + 1) begin
            fork
                send_byte(test_bytes[i]);
                recv_byte(received);
            join
            if (received === test_bytes[i]) begin
                $display("PASS byte[%0d] = 0x%02X '%c'", i, test_bytes[i], test_bytes[i]);
                pass_count = pass_count + 1;
            end else begin
                $display("FAIL byte[%0d]: sent=0x%02X got=0x%02X", i, test_bytes[i], received);
                fail_count = fail_count + 1;
            end
            // Gap between bytes
            repeat(5) @(posedge clk);
        end

        $display("---");
        $display("uart-echo: %0d PASS, %0d FAIL", pass_count, fail_count);
        if (fail_count == 0) $display("ALL PASS");
        else                 $display("FAILURES DETECTED");
        $finish;
    end

    // Timeout
    initial begin
        #10_000_000;
        $display("TIMEOUT");
        $finish;
    end

endmodule
