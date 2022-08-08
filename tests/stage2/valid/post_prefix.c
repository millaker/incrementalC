int main(){
    int a = 5;
    int b = 10;
    b += a++;
    b += ++a;
    b += --a;
    b += a--;
    return a + b++;
}
