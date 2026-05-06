int Add(int a, int b) {
    int result = a + b;
    return result;
}

int main() {
    int x = 10;
    int y = 0;

    if (x > 0) {
        y = Add(x, 5);
    } else {
        y = -1;
    }

    for (int i = 0; i < 3; i++) {
        y += i;
    }

    while (y < 20) {
        y++;
    }

    do {
        y--;
    } while (y > 10);

    {
        int scopedValue = y * 2;
        y = scopedValue;
    }

    return y;
}
