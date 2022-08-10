int main() {
    int fibo = 1;
    int second = 1;
    int iterations = 15; 
    for(int i = 0; i < iterations - 1; i++){
        int temp = fibo + second;
        fibo = second;
        second = temp;
    }
    return fibo;
}

