






struct Pos
{
    int x;
    int y;

    // default + parameterized constructor
    Pos(int x=0, int y=0) 
        : x(x), y(y)
    {
    }

    // assignment operator modifies object, therefore non-const
    Pos& operator=(const Pos& a)
    {
        x=a.x;
        y=a.y;
        return *this;
    }

    // addop. doesn't modify object. therefore const.
    Pos operator+(const Pos& a) const
    {
        return Pos(a.x+x, a.y+y);
    }

    // equality comparison. doesn't modify object. therefore const.
    bool operator==(const Pos& a) const
    {
        return (x == a.x && y == a.y);
    }
	
	bool operator!=(const Pos& a) const
    {
        return (x != a.x || y != a.y);
    }
	
};