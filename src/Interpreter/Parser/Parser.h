class Parser{
public:
    void generateParseTree(const std::vector<Token> & token_stream);

private:
    ASTree astree_;
}