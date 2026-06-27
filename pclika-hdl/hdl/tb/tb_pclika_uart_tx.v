/**
 * tb_pclika_uart_tx.v — UART TX testbench (loopback via RX)
 *
 * Connects pclika_uart_tx → pclika_uart_rx (wire loopback).
 * Sends bytes 0x55, 0xA3, 0x00, 0xFF and verifies rx receives them.
 *
 * Uses reduced BAUD_DIV=10 for fast simulation.
 */

`timescale 1ns / 1ps

module tb_pclika_uart_tx;

    localparam CLK_FREQ  = 12_000_000;
    localparam BAUD_RATE = 1_200_000;  // BAUD_DIV=10 for speed
    localparam DATA_BITS = 8;

    reg clk = 0, rst_n = 0;
    always #41 clk = ~clk;   // ~12 MHz

    // DUT wires
    reg  [DATA_BITS-1:0] tx_data;
    reg                  tx_valid;
    wire                 tx_line;
    wire                 tx_busy;
    wire [DATA_BITS-1:0] rx_data;
    wire                 rx_valid;
    wire                 rx_framing_err;

    // TX
    pclika_uart_tx #(
        .CLK_FREQ (CLK_FREQ),
        .BAUD_RATE(BAUD_RATE),
        .DATA_BITS(DATA_BITS)
    ) u_tx (
        .clk       (clk),
        .rst_n     (rst_n),
        .data      (tx_data),
        .data_valid(tx_valid),
        .tx        (tx_line),
        .busy      (tx_busy)
    );

    // RX — loopback from TX output
    pclika_uart_rx #(
        .CLK_FREQ (CLK_FREQ),
        .BAUD_RATE(BAUD_RATE),
        .DATA_BITS(DATA_BITS)
    ) u_rx (
        .clk        (clk),
        .rst_n      (rst_n),
        .rx         (tx_line),
        .data       (rx_data),
        .data_valid (rx_valid),
        .framing_err(rx_framing_err)
    );

    // Test data
    reg [DATA_BITS-1:0] expected [0:3];
    integer pass_count;
    integer fail_count;
    integer i;

    initial begin
        expected[0] = 8'h55;
        expected[1] = 8'hA3;
        expected[2] = 8'h00;
        expected[3] = 8'hFF;
        pass_count  = 0;
        fail_count  = 0;

        $dumpfile("tb_pclika_uart_tx.vcd");
        $dumpvars(0, tb_pclika_uart_tx);

        // Reset
        rst_n    = 0;
        tx_data  = 0;
        tx_valid = 0;
        repeat(5) @(posedge clk);
        rst_n = 1;
        @(posedge clk);

        // Send each byte; wait for rx_valid
        for (i = 0; i < 4; i = i + 1) begin
            // Load and pulse tx_valid
            @(negedge clk);
            tx_data  = expected[i];
            tx_valid = 1'b1;
            @(posedge clk);
            #1;
            tx_valid = 1'b0;

            // Wait for RX to capture
            wait (rx_valid === 1'b1);
            @(posedge clk);

            // Verify
            if (rx_data === expected[i]) begin
                $display("PASS byte[%0d]: tx=0x%02X rx=0x%02X", i, expected[i], rx_data);
                pass_count = pass_count + 1;
            end else begin
                $display("FAIL byte[%0d]: expected=0x%02X got=0x%02X", i, expected[i], rx_data);
                fail_count = fail_count + 1;
            end
            if (rx_framing_err) begin
                $display("FAIL byte[%0d]: framing error", i);
                fail_count = fail_count + 1;
            end

            // Wait for TX idle before next
            wait (!tx_busy);
            repeat(3) @(posedge clk);
        end

        // Summary
        $display("---");
        $display("UART TX/RX loopback: %0d PASS, %0d FAIL", pass_count, fail_count);
        if (fail_count == 0)
            $display("ALL PASS");
        else
            $display("FAILURES DETECTED");

        $finish;
    end

    // Timeout guard
    initial begin
        #5_000_000;
        $display("TIMEOUT");
        $finish;
    end

endmodule
