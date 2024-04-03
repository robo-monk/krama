def rule110(current_gen):
    """Applies Rule 110 to the current generation of cells and returns the next generation."""
    next_gen = []
    # Pad the ends of the current generation with zeros to handle edge cells
    padded_gen = [0] + current_gen + [0]
    for i in range(1, len(padded_gen) - 1):
        neighbors = (padded_gen[i-1], padded_gen[i], padded_gen[i+1])
        # Translate the tuple of neighbors into the next state according to Rule 110
        next_state = 0
        if neighbors in [(1, 1, 1), (1, 0, 0), (0, 0, 0)]:
            next_state = 0
        else:
            next_state = 1
        next_gen.append(next_state)
    return next_gen

def print_generation(gen):
    """Prints the current generation of cells."""
    print(''.join(['#' if cell == 1 else '.' for cell in gen]))

def simulate_rule110(width, generations):
    """Simulates Rule 110 starting with a single 1 in the middle of the first generation."""
    current_gen = [0] * width
    # Start with a single 1 in the middle
    current_gen[width // 2] = 1
    print_generation(current_gen)
    
    for _ in range(generations - 1):
        current_gen = rule110(current_gen)
        print_generation(current_gen)

# Simulate Rule 110 with a width of 31 cells for 15 generations
simulate_rule110(31, 15)

