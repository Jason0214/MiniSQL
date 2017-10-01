class Parser{
public:
    void generateParseTree(const std::vector<Token> & token_stream);

private:
    ParserTree tree_to_exec_;
    AliasMap alias_map_;

}