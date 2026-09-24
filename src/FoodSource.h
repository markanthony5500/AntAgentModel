// Defines food source for ants
struct FoodSource {
    int x, y;
    int amount;
    float toxicity;
    bool isDepleted(){
        if(amount <= 0){return true;}
        else{
            return false;
        }
    }

    bool take(){
        if(isDepleted()){
            return false;
        }
        else{
            --amount;
        }
    }
};

