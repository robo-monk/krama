// Define the struct for a Node in the list
struct Node<T> {
    data: T,
    next: Option<Box<Node<T>>>,
    prev: Option<Box<Node<T>>>,
}

// Define the LinkedList struct
struct LinkedList<T> {
    head: Option<Box<Node<T>>>,
    tail: Option<Box<Node<T>>>,
}

// Implement LinkedList
impl<T> LinkedList<T> {
    // Create a new empty list
    fn new() -> Self {
        LinkedList { head: None, tail: None }
    }

    // Push a new element to the front of the list
    fn push(&mut self, data: T) {
        let new_node = Box::new(Node {
            data,
            next: self.head.take(),
			prev: None
        });
		self.head.prev = Some(new_node);

        self.head = Some(new_node);
    }

    // Pop an element from the front of the list
    fn pop(&mut self) -> Option<T> {
        self.head.take().map(|node| {
            self.head = node.next;
            node.data
        })
    }

    // Peek at the front element of the list, without removing it
    fn peek(&self) -> Option<&T> {
        self.head.as_ref().map(|node| &node.data)
    }
}

fn main() {
    let mut list = LinkedList::new();

    list.push(1);
    list.push(2);
    list.push(3);

    println!("{:?}", list.pop()); // Output: Some(3)
    println!("{:?}", list.peek()); // Output: Some(&2)
}

