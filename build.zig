const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});

    // Change the target platform here if needed
    const target = b.resolveTargetQuery(std.zig.CrossTarget{
        .cpu_arch = .aarch64,
        .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_a53 },
        .os_tag = .freestanding,
        .abi = .none,
        .ofmt = .elf,
    });

    const FiberFlow_deps = b.dependency("FiberFlow", .{
        .optimize = optimize,
    });

    const lib = b.addStaticLibrary(.{
        .name = "FiberPool",
        .target = target,
        .optimize = optimize,
        // For library, we should always maintain the Symbol Table
        .strip = false,
    });

    const FiberFlow = FiberFlow_deps.artifact("FiberFlow");

    b.installArtifact(FiberFlow);

    lib.linkLibrary(FiberFlow);

    lib.addIncludePath(.{ .path = "." });

    lib.addCSourceFile(.{ .file = b.path("FiberPool.c") });

    b.installArtifact(lib);
}
