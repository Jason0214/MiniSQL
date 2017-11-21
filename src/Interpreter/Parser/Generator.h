#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include <stack>

#include "../Lexer/Lexer.h"
#include "ASTree.h"
#include "ASTreeNodeStack.h"
#include "ParserSymbol.h"

namespace Generator{
    class QueryGenerator{
    public:
        QueryGenerator(){
            this->generate_funcs_[0] = NULL;
            this->generate_funcs_[1] = &wait_select;
            this->generate_funcs_[2] = &wait_attr_id;
            this->generate_funcs_[3] = &reduce_attr_id;
            this->generate_funcs_[4] = &wait_addr_dot_right;
            this->generate_funcs_[5] = &reduce_attr_id_with_table_id;
            this->generate_funcs_[6] = &reduce_attr;
            this->generate_funcs_[7] = &wait_attr_alias;
            this->generate_funcs_[8] = &reduce_attr_with_alias;
            this->generate_funcs_[9] = &reduce_attr_set;
            this->generate_funcs_[10] = &wait_from;
            this->generate_funcs_[11] = &wait_table_id;
            this->generate_funcs_[12] = &reduce_table_id;
            this->generate_funcs_[13] = &reduce_table;
            this->generate_funcs_[14] = &wait_table_alias;
            this->generate_funcs_[15] = &reduce_table_with_alias;
            this->generate_funcs_[16] = &reduce_table_set;
            this->generate_funcs_[17] = &wait_where;
            this->generate_funcs_[18] = &reduce_query_without_condition;
            this->generate_funcs_[19] = &wait_condition;
            this->generate_funcs_[20] = &wait_num_or_str;
            this->generate_funcs_[21] = &wait_equality;
            this->generate_funcs_[22] = &reduce_condition;
            this->generate_funcs_[23] = &reduce_condition_set;
            this->generate_funcs_[24] = &reduce_query_with_condition;
        }
        ~QueryGenerator(){}
        void Accept(TokenStream & token_stream, ASTNodeStack & s){
            ParserSymbol::QueryState state = ParserSymbol::WAIT_SELECT;
            while(state != ParserSymbol::FINISH_QUERY){
                int index = state2index(state);
                state = (*(this->generate_funcs_[index]))(token_stream, s);
            }
        }
    private:
        static int state2index(ParserSymbol::QueryState st){
            return (int)st;
        }

        ParserSymbol::QueryState (*generate_funcs_[QUERY_STATE_CNT])(TokenStream & token_stream, ASTNodeStack & s);

        static ParserSymbol::QueryState wait_select(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_attr_id(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_attr_id(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_addr_dot_right(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_attr_id_with_table_id(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_attr(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_attr_alias(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_attr_with_alias(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_attr_set(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_from(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_table_id(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_table_id(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_table(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_table_alias(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_table_with_alias(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_table_set(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_where(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_query_without_condition(TokenStream & token_stream, ASTNodeStack & s); 
        static ParserSymbol::QueryState wait_condition(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_num_or_str(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState wait_equality(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_condition(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_condition_set(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::QueryState reduce_query_with_condition(TokenStream & token_stream, ASTNodeStack & s);
    };

    class DeleteGenerator{
    public:
        DeleteGenerator(){
            this->generate_funcs_[0] = NULL;
            this->generate_funcs_[1] = &wait_delete;
            this->generate_funcs_[2] = &wait_from_in_delete;
            this->generate_funcs_[3] = &wait_table_id_in_delete;
            this->generate_funcs_[4] = &wait_where_in_delete;
            this->generate_funcs_[5] = &wait_condition_in_delete;
            this->generate_funcs_[6] = &wait_attr_in_delete;
            this->generate_funcs_[7] = &wait_num_or_str_in_delete;
            this->generate_funcs_[8] = &wait_equality_in_delete;
            this->generate_funcs_[9] = &reduce_condition_in_delete;
            this->generate_funcs_[10] = &reduce_condition_set_in_delete;
            this->generate_funcs_[11] = &reduce_delete;
        }
        ~DeleteGenerator(){}
        void Accept(TokenStream & token_stream, ASTNodeStack & s){
            ParserSymbol::DeleteState state = ParserSymbol::WAIT_DELETE;
            while(state != ParserSymbol::FINISH_DELETE){
                int index = state2index(state);
                state = (*(this->generate_funcs_[index]))(token_stream, s);
            }
        }
    private:
        static int state2index(ParserSymbol::DeleteState st){
            return (int)st;
        }

        ParserSymbol::DeleteState (*generate_funcs_[DELETE_STATE_CNT])(TokenStream & token_stream, ASTNodeStack & s);
       
        static ParserSymbol::DeleteState wait_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState wait_from_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState wait_table_id_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState wait_where_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState wait_condition_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState wait_attr_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState wait_num_or_str_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState wait_equality_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState reduce_condition_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState reduce_condition_set_in_delete(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::DeleteState reduce_delete(TokenStream & token_stream, ASTNodeStack & s);
    };

    class InsertGenerator{
    public:
        InsertGenerator(){
            this->generate_funcs_[0] = NULL;
            this->generate_funcs_[1] = &wait_insert;
            this->generate_funcs_[2] = &wait_into;
            this->generate_funcs_[3] = &wait_table_in_insert;
            this->generate_funcs_[4] = &wait_value_set;
            this->generate_funcs_[5] = &wait_single_value;
            this->generate_funcs_[6] = &begin_of_value_set;
            this->generate_funcs_[7] = &reduce_value_set;
        }
        ~InsertGenerator(){}
        void Accept(TokenStream & token_stream, ASTNodeStack & s){
            ParserSymbol::InsertState state = ParserSymbol::WAIT_INSERT;
            while(state != ParserSymbol::FINISH_INSERT){
                int index = state2index(state);
                state = (*(this->generate_funcs_[index]))(token_stream, s);
            }
        }
    private:
        static int state2index(ParserSymbol::InsertState st){
            return (int)st;
        }
        
        ParserSymbol::InsertState (*generate_funcs_[INSERT_STATE_CNT])(TokenStream & token_stream, ASTNodeStack & s);
       
        static ParserSymbol::InsertState wait_insert(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::InsertState wait_into(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::InsertState wait_table_in_insert(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::InsertState wait_value_set(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::InsertState wait_single_value(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::InsertState begin_of_value_set(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::InsertState reduce_value_set(TokenStream & token_stream, ASTNodeStack & s);
    };

    class CreateTableGenerator{
    public:
        CreateTableGenerator(){
            this->generate_funcs_[0] = NULL;
            this->generate_funcs_[1] = &wait_table_in_create_table;
            this->generate_funcs_[2] = &begin_of_meta_set;
            this->generate_funcs_[3] = &wait_meta;
            this->generate_funcs_[4] = &wait_type;
            this->generate_funcs_[5] = &wait_type_param;
            this->generate_funcs_[6] = &reduce_type;
            this->generate_funcs_[7] = &wait_constrain;
            this->generate_funcs_[8] = &reduce_meta;
            this->generate_funcs_[9] = &reduce_meta_set;
        }
        ~CreateTableGenerator(){}
        void Accept(TokenStream & token_stream, ASTNodeStack & s){
            ParserSymbol::CreateTableState state = ParserSymbol::WAIT_TABLE_IN_CREATE_TABLE;
            while(state != ParserSymbol::FINISH_CREATE_TABLE){
                int index = state2index(state);
                state = (*(this->generate_funcs_[index]))(token_stream, s);
            }
        }
    private:
        static int state2index(ParserSymbol::CreateTableState st){
            return (int)st;
        }
        
        ParserSymbol::CreateTableState (*generate_funcs_[CREATE_TABLE_STATE_CNT])(TokenStream & token_stream, ASTNodeStack & s);

        static ParserSymbol::CreateTableState wait_table_in_create_table(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState begin_of_meta_set(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState wait_meta(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState wait_type(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState wait_type_param(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState reduce_type(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState wait_constrain(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState reduce_meta(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::CreateTableState reduce_meta_set(TokenStream & token_stream, ASTNodeStack & s);
    };
    
    class UpdateGenerator{
    public:
        UpdateGenerator(){
            this->generate_funcs_[0] = NULL;
            this->generate_funcs_[1] = &wait_update;
            this->generate_funcs_[2] = &wait_table_in_update;
            this->generate_funcs_[3] = &begin_of_assign_set;
            this->generate_funcs_[4] = &wait_attr_in_assign;
            this->generate_funcs_[5] = &wait_equal_in_assign;
            this->generate_funcs_[6] = &wait_assign_value;
            this->generate_funcs_[7] = &reduce_assign;
            this->generate_funcs_[8] = &reduce_assign_set;
        }
        ~UpdateGenerator(){}
        void Accept(TokenStream & token_stream, ASTNodeStack & s){
            ParserSymbol::UpdateState state = ParserSymbol::WAIT_UPDATE;
            while(state != ParserSymbol::FINISH_UPDATE){
                int index = state2index(state);
                state = (*(this->generate_funcs_[index]))(token_stream, s);
            } 
        }
    private:
        static int state2index(ParserSymbol::UpdateState st){
            return (int)st;
        }
        
        ParserSymbol::UpdateState (*generate_funcs_[UPDATE_STATE_CNT])(TokenStream & token_stream, ASTNodeStack & s);
  
        static ParserSymbol::UpdateState wait_update(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::UpdateState wait_table_in_update(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::UpdateState begin_of_assign_set(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::UpdateState wait_attr_in_assign(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::UpdateState wait_equal_in_assign(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::UpdateState wait_assign_value(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::UpdateState reduce_assign(TokenStream & token_stream, ASTNodeStack & s);
        static ParserSymbol::UpdateState reduce_assign_set(TokenStream & token_stream, ASTNodeStack & s);
    };
};

#endif