int main(){
    int num1 = 96;
    int num2 = 84;
    int i = num1 > num2 ? num2 : num1;
    while(i > 0){
        if((num1 % i) == 0 && (num2 % i) == 0)
            break;
        i--;
    }
    return i;
}
