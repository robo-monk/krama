import testing_utils as tu

program = """
def |row: ptr| print_grid() {
    let len = row:length()
    #c for (int i = 1; i < len; i++) {if(row[i]){printf("#");}else{printf(".");}};
	#c printf("\\n")
}

tree is_alive(ary: ptr, idx: i32) {
    (idx < 0) -> { 0:as_i32() }
    (idx > (ary:length() - 1)) -> { 0:as_i32() }
    () -> { ary:at(idx) }
}

tree state_from_neighbours (lcell: i32, ccell: i32, rcell: i32) {
    (lcell && ccell && rcell)                   -> { 0:as_i32() }
    (lcell && ccell && (rcell < 1))     		-> { 1:as_i32() }
    (lcell && (ccell < 1) && rcell)     	    -> { 1:as_i32() }
    (lcell && (ccell < 1) && (rcell < 1))  		-> { 0:as_i32() }
    ((lcell < 1) && ccell && rcell)  			-> { 1:as_i32() }
    ((lcell < 1) && ccell && (rcell < 1))  		-> { 1:as_i32() }
    ((lcell < 1) && (ccell < 1) && rcell)  		-> { 1:as_i32() }
    ((lcell < 1) && (ccell < 1) && (rcell < 1)) -> { 0:as_i32() }
}


def |ary: ptr| rule110(idx: i32) {
    let lcell: i32 = is_alive(ary, idx - 1)
   	let ccell: i32 = is_alive(ary, idx)
    let rcell: i32 = is_alive(ary, idx + 1)
    let s = state_from_neighbours(lcell, ccell, rcell)
	s
}

def |ary: ptr| apply_rule110(idx: i32, previous: ptr) {
		if (idx < ary:length()) {
			let new_state = previous:rule110(idx)
			ary:set(idx, new_state)
			ary:apply_rule110(idx + 1:as_i32(), previous)
		}
		#
}

def iterate_rule110(ary: ptr, iterations: i32) {
	if (iterations > 0) {
		let cloned = ary:clone()
 		ary:apply_rule110(0, cloned)
		drop(cloned)
    ary:print_grid()
		iterate_rule110(ary, iterations - 1)
	}
	#
}

def main() {
    let row = array(31):fill(0, 0)
		row:set(29, 1)
    row:print_grid()
		iterate_rule110(row, 42)
    0:as_i32()
}
"""

expected_out ="""
.............................#
............................##
...........................###
..........................##.#
.........................#####
........................##...#
.......................###..##
......................##.#.###
.....................#######.#
....................##.....###
...................###....##.#
..................##.#...#####
.................#####..##...#
................##...#.###..##
...............###..####.#.###
..............##.#.##..#####.#
.............########.##...###
............##......####..##.#
...........###.....##..#.#####
..........##.#....###.####...#
.........#####...##.###..#..##
........##...#..#####.#.##.###
.......###..##.##...########.#
......##.#.######..##......###
.....#######....#.###.....##.#
....##.....#...####.#....#####
...###....##..##..###...##...#
..##.#...###.###.##.#..###..##
.#####..##.###.######.##.#.###
##...#.#####.###....########.#
##..####...###.#...##......###
##.##..#..##.###..###.....##.#
#####.##.#####.#.##.#....#####
#...######...########...##...#
#..##....#..##......#..###..##
#.###...##.###.....##.##.#.###
###.#..#####.#....##########.#
#.###.##...###...##........###
###.####..##.#..###.......##.#
#.###..#.#####.##.#......#####
###.#.####...######.....##...#
#.#####..#..##....#....###..##
###...#.##.###...##...##.#.###
"""

# tu.expect_in_out_executed(program, expected_out)
(stdout, stderr, result) = tu.compile_and_run(program)
assert stdout is not None
assert stdout.replace("\n","").strip() == expected_out.replace("\n", "").strip()
