#include "val.h"

Value Value::operator+(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(Itemp + op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(Rtemp + op.GetReal());
    }
    if (IsString() && op.IsString()) {
        return Value(Stemp + op.GetString());
    }
    if (IsString() && op.IsChar()) {
        return Value(Stemp + string(1, op.GetChar()));
    }
    if (IsChar() && op.IsChar()) {
        return Value(Ctemp + op.GetChar());
    }
    if (IsChar() && op.IsString()) {
        return Value(string(1, Ctemp) + op.GetString());
    }
    else {
        return Value();
    }
}

Value Value::operator-(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(Itemp - op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(Rtemp - op.GetReal());
    }
    else {
        return Value();
    }
}

Value Value::operator*(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(Itemp * op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(Rtemp * op.GetReal());
    }
    else {
        return Value();
    }
}
    
Value Value::operator/(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        if (op.GetInt() == 0) {
            return Value();
        }
        return Value(Itemp / op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        if (op.GetReal() == 0.0) {
            return Value();
        }
        return Value(Rtemp / op.GetReal());
    }
    else {
        return Value();
    }
}
    
Value Value::operator%(const Value & op) const {
    if (IsInt() && op.IsInt()) {
        if (op.GetInt() == 0) {
            return Value();
        }
        return Value(Itemp % op.GetInt());
    }
    else {
        return Value();
    }
}
         
Value Value::operator==(const Value& op) const {
    if (GetType() != op.GetType()) {
        return Value(false);
    }
    if (IsInt()) {
        return Value(Itemp == op.GetInt());
    }
    if (IsReal()) {
        return Value(Rtemp == op.GetReal());
    }
    if (IsBool()) {
        return Value(Btemp == op.GetBool());
    }
    if (IsString()) {
        return Value(Stemp == op.GetString());
    }
    if (IsChar()) {
        return Value(Ctemp == op.GetChar());
    }
    return Value(false);
}
    
Value Value::operator!=(const Value& op) const {
    Value eq = (*this == op);
    return Value(!eq.GetBool());
}

Value Value::operator>(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(Itemp > op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(Rtemp > op.GetReal());
    }
    if (IsString() && op.IsString()) {
        return Value(Stemp > op.GetString());
    }
    if (IsChar() && op.IsChar()) {
        return Value(Ctemp > op.GetChar());
    }
    else {
        return Value();
    }
}

Value Value::operator<(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        return Value(Itemp < op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        return Value(Rtemp < op.GetReal());
    }
    if (IsString() && op.IsString()) {
        return Value(Stemp < op.GetString());
    }
    if (IsChar() && op.IsChar()) {
        return Value(Ctemp < op.GetChar());
    }
    else {
        return Value();
    }
}

Value Value::operator<=(const Value& op) const {
    return (*this < op) || (*this == op);
}

Value Value::operator>=(const Value& op) const {
    return (*this > op) || (*this == op);
}

Value Value::operator&&(const Value& op) const {
    if (IsBool() && op.IsBool()) {
        return Value(Btemp && op.GetBool());
    }
    else {
        return Value();
    }
}

Value Value::operator||(const Value& op) const {
    if (IsBool() && op.IsBool()) {
        return Value(Btemp || op.GetBool());
    }
    else {
        return Value();
    }
}

Value Value::operator!(void) const {
    if (IsBool()) {
        return Value(!Btemp);
    }
    else {
        return Value();
    }
}

Value Value::Concat(const Value& op) const {
    if (IsString() && op.IsString()) {
        return Value(Stemp + op.GetString());
    }
    if (IsString() && op.IsChar()) {
        return Value(Stemp + string(1, op.GetChar()));
    }
    if (IsChar() && op.IsString()) {
        return Value(string(1, Ctemp) + op.GetString());
    }
    if (IsChar() && op.IsChar()) {
        return Value(string(1, Ctemp) + string(1, op.GetChar()));
    }
    else {
        return Value();
    }
}

Value Value::Exp(const Value& op) const {
    if (!IsReal() || !op.IsReal()) {
        return Value();
    }
    double base = Rtemp;
    double exponent = op.GetReal();
    
    if (base == 0.0 && exponent < 0) {
        return Value();
    }
    return Value(pow(base, exponent));
}