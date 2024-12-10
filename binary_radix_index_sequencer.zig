pub const std = @import("std");
pub const allocator = std.heap.page_allocator;

pub const RString = struct {
    data: std.ArrayList(usize),
    radix: usize = 10,
    size: usize = 0,
    last: usize = 0,

    pub fn init() RString {
        return RString{
            .data = std.ArrayList(usize).init(allocator),
        };
    }

    pub fn push(self: *RString, c: usize) !void {
        try self.data.append(c);
    }

    pub fn unshift(self: *RString, c: usize) !void {
        try self.data.insert(0, c);
    }

    pub fn double(self: *RString) !void {
        var carry: usize = 0;
        for (self.data.items) |*v| {
            const value = (v.* * 2) + carry;
            v.* = value % self.radix;
            carry = value / self.radix;
        }
        if (carry != 0) {
            try self.push(carry);
        }
    }

    pub fn doubleTill(self: *RString) !void {
        var carry: usize = 0;
        for (self.data.items) |*v| {
            const value = (v.* * 2) + carry;
            v.* = value % self.radix;
            carry = value / self.radix;
        }
        if (carry != 0 and self.data.items.len <= self.size) {
            try self.push(carry);
        }
    }

    pub fn print(self: *RString) !void {
        for (self.data.items) |c| {
            std.debug.print("{d}", .{c});
        }
        std.debug.print("\n", .{});
    }

    pub fn write(self: *RString, writer_: anytype) !void {
        var buffer = try std.fmt.allocPrint(allocator, "{any}", .{self.data.items});
        try writer_.writeAll(buffer);
    }
};

pub fn find_repeat(nth: *RString, length: *usize, start: *usize) bool {
    const nlen: usize = nth.data.items.len;
    const n2: usize = nlen / 2;
    // std.debug.print("nlen: {d}\n", .{nlen});
    // std.debug.print("n2: {d}\n", .{n2});
    // std.debug.print("nth: {any}\nlen:{any}\n", .{ nth.data.items, nth.data.items.len });
    for ((nth.last)..n2) |seq_length| {
        if (seq_length == 0) continue;
        name: for (0..n2) |seq_start| {
            // number of sequences that could fit given our start and length
            const num_seq = (nth.data.items.len - seq_start) / seq_length;
            if (num_seq < 2) {
                break;
            }

            // make a slice of the sequence we are looking for
            const seq: []usize = nth.data.items[seq_start .. seq_start + seq_length];

            for (1..num_seq) |i| {
                if (seq_start + (i * seq_length) + seq_length > nth.data.items.len) {
                    break;
                }
                // make a slice of the sequence we are comparing
                const compare: []usize = nth.data.items[seq_start + (i * seq_length) .. (seq_start + i * seq_length) + seq_length];
                if (!std.mem.eql(usize, seq, compare)) {
                    continue :name;
                }
            }
            // std.debug.print("Found repeating sequence\n{any}\n", .{seq});

            // if we made it here, we found a repeating sequence
            length.* = seq_length;
            start.* = seq_start;
            return true;
        }
    }
    return false;
}

pub fn find_nth_sequence(n: usize, radix: usize) !RString {
    var dec: RString = RString.init();
    dec.radix = radix;
    var result: RString = RString.init();
    result.radix = radix;
    var nth_sequence: RString = RString.init();
    nth_sequence.radix = radix;
    defer {
        dec.data.deinit();
        nth_sequence.data.deinit();
    }
    dec.size = n;

    try dec.push(1);
    for (0..n) |_| {
        try dec.push(0);
    }

    for (0..100) |_| {
        try nth_sequence.push(dec.data.items[n]);
        try dec.double();
    }

    var found: bool = false;
    var length: usize = 0;
    var start: usize = 0;
    var i: usize = 0;
    found = find_repeat(&nth_sequence, &length, &start);
    while (!found) {
        if (i > 1000) {
            return error.ToManyIterations;
        }
        // std.debug.print("iter: {d} len: {d}\r", .{ i, dec.data.items.len });
        i += 1;
        for (0..100 * (n + 1) * (n + 1)) |_| {
            try nth_sequence.push(dec.data.items[n]);
            try dec.double();
        }
        found = find_repeat(&nth_sequence, &length, &start);
        nth_sequence.last = nth_sequence.data.items.len / 2;
    }

    for (0..length) |ix| {
        try result.push(nth_sequence.data.items[start + ix]);
    }

    // std.debug.print("Result: \n", .{});
    // try result.print();

    // std.debug.print("Length: \n{any}", .{length});
    // std.debug.print("\n", .{});
    // std.debug.print("inside : \n{any}", .{nth_sequence.data.items});
    result.size = length;
    return result;
}

pub fn find_nth_len_fast(n: usize, radix: usize) !u256 {
    var lengthStr = try find_nth_sequence(0, radix);
    var length: u256 = lengthStr.size;
    if (n == 0) return length;
    lengthStr.data.deinit();

    var n_no_2_prime_factors = radix;
    while (n_no_2_prime_factors % 2 == 0) {
        n_no_2_prime_factors = n_no_2_prime_factors >> 1;
    }
    for (0..n) |_| {
        if (length >= std.math.maxInt(u250) / 16) {
            return error.SequenceTooLong;
        }
        length = length * n_no_2_prime_factors;
    }

    return length;
}
pub fn find_nth_sequence_fast(n: usize, radix: usize) !RString {
    var lengthStr = try find_nth_sequence(0, radix);
    var length = lengthStr.size;
    if (n == 0) return lengthStr;
    lengthStr.data.deinit();

    var n_no_2_prime_factors = radix;
    while (n_no_2_prime_factors % 2 == 0) {
        n_no_2_prime_factors = n_no_2_prime_factors >> 1;
    }
    for (0..n) |_| {
        if (length >= std.math.maxInt(usize) / 16) {
            return error.SequenceTooLong;
        }
        length = length * n_no_2_prime_factors;
        // std.debug.print("length: {d}\n", .{length});
    }

    var dec: RString = RString.init();
    dec.size = n;
    dec.radix = radix;
    var result: RString = RString.init();
    result.radix = radix;
    defer dec.data.deinit();

    try dec.push(1);
    for (0..n) |_| {
        try dec.push(0);
    }

    for (0..(length)) |i| {
        try result.push(dec.data.items[n]);
        try dec.doubleTill();
        if (i % (100 * n * radix) == 0) std.debug.print("Index{d} of {d}: {d}/{d}\r", .{ n, radix, i, length });
    }
    std.debug.print("\n", .{});

    // std.debug.print("Result: \n", .{});
    // try result.print();

    // std.debug.print("Length: \n{any}", .{length});
    // std.debug.print("\n", .{});
    return result;
}

pub fn main() !void {
    for (2..1000) |i| {
        // open a new file for writing with the filename seq_{i}.numbers
        const filename = try std.fmt.allocPrint(allocator, "seq/seq_{d}.numbers", .{i});
        var file = try std.fs.cwd().createFile(filename, .{});
        defer file.close();
        var writer = file.writer();

        for (0..100) |j| {
            var length = find_nth_len_fast(j, i) catch break;
            const j_str = try std.fmt.allocPrint(allocator, "{d},{d}:", .{ j, length });
            try writer.writeAll(j_str);
            if (length < 6000000) {
                var seq = find_nth_sequence_fast(j, i) catch break;
                seq.write(writer) catch break;
            }
            try writer.writeByte('\n');
        }
    }
    _ = try find_nth_sequence_fast(60, 2);
    // 0,2,
}
