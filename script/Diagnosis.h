#pragma once

#include "lexer.h"
#include <string>

namespace script
{
    enum class DiagType
    {
        DT_Error,
        DT_Warning,
        DT_Tips,
    };

    class Diagnosis
    {
        friend class DiagnosisConsumer;
    public:
        Diagnosis(DiagType type, TokenCoord coord)
            : type_(type), coord_(coord) {}

        Diagnosis &operator << (const char * str)
        {
            message_ += str;
            return *this;
        }

        Diagnosis &operator << (char c)
        {
            message_ += c;
            return *this;
        }

        Diagnosis &operator << (const std::string &str)
        {
            message_ += str;
            return *this;
        }

        Diagnosis &operator << (int integer)
        {
            // TODO:
            return *this;
        }
        
        static const char *TokenToStirng(unsigned kind);

        static const char *DiagTypeToString(DiagType type);

    private:
        std::string format();

    private:
        DiagType type_;
        TokenCoord coord_;
        std::string message_;
    };
}