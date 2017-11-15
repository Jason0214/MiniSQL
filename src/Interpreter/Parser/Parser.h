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
    void clear(){
        this->astree_.destroy();
    }
private:
    Parser(const Parser &);
    const Parser & operator=(const Parser &);

    void loadGenerator();
    void deleteGenerator();

    void parseCreateSentence(TokenStream & token_stream);
    void parseDropSentence(TokenStream & token_stream);
    void parseSelectSentence(TokenStream & token_stream);
    void parseDeleteSentence(TokenStream & token_stream);
    void parseDropIndexSentence(TokenStream & token_stream);
    void parseCreateIndexSentence(TokenStream & token_stream);
    void parseInsertSentence(TokenStream & token_stream);
    void parseUpdateSentence(TokenStream & token_stream);
    void parseCreateTableSentence(TokenStream & token_stream);
    void parseDropTableSentence(TokenStream & token_stream);


    Generator::QueryGenerator* getQueryGenerator(ParserSymbol::QueryState state);
    Generator::DeleteGenerator* getDeleteGenerator(ParserSymbol::DeleteState state);

    Generator::QueryGenerator* query_generators_[QUERY_STATE_CNT];
    Generator::DeleteGenerator* delete_generators_[DELETE_STATE_CNT];

    ASTree astree_;
};

#endif