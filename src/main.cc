#include "../include/BPlusTree.h"
#include <iostream>

int main() {
    BPlusTree tree(3);

    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(5, 50);
    tree.insert(6, 60);
    tree.insert(15, 150);

    tree.print();

    auto result = tree.search(6);

    if (result) {
        std::cout << "Found: " << *result << "\n";
    } else {
        std::cout << "Not found\n";
    }

    tree.remove(6);
    tree.print();

    tree.remove(5);
    tree.print();
}
