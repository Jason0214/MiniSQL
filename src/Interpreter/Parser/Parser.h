#ifndef __PARSER_H__
#define __PARSER_H__

#include "ASTree.h"
#include "Generator.h"
#include "ParserSymbol.h"


class Parser{
public:
    Parser(){
        this->loadGenerator();
    }
    ~Parser(){
        this->deleteGenerator();
    };
    void parseSentence(TokenStream & token_stream);

    const ASTree & getASTree() const{
        return this->astree_;
    }
private:
    Parser(const Parser &);
    const Parser & operator=(const Parser &);

    void loadGenerator();
    void deleteGenerator();
    Generator::ASTgenerator* getGenerator(ParserSymbol::SLRstate state);
    
    void parseSelectSentence(TokenStream & token_stream);

    Generator::ASTgenerator* generators_[GENERATOR_CNT];
    ASTree astree_;
};

#endif