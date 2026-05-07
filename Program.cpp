int Add(int a, int b) {
    return a + b;
}

void PrintValue(int value) {
}

int main() {
    int x = 10;
    int y = 20;

    int result = Add(x, y);

    if (result > 20) {
        PrintValue(result);
    } else {
        PrintValue(0);
    }

    for (int i = 0; i < 3; i++) {
        result += i;
        PrintValue(result);
    }

    while (result < 40) {
        result++;
    }

    do {
        result--;
    } while (result > 30);

    return 0;
}
