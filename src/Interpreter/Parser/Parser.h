#ifndef __PARSER_H__
#define __PARSER_H__

#include "ASTree.h"
#include "Generator.h"
#include "ParserSymbol.h"


class Parser{
public:
    Parser(){}
    ~Parser(){};
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

    Generator::QueryGenerator query_generator_;
    Generator::DeleteGenerator delete_generator_;
    Generator::InsertGenerator insert_generator_;
    Generator::CreateTableGenerator create_table_generator_;
    Generator::UpdateGenerator update_generator_;

    ASTree astree_;
};

#endif