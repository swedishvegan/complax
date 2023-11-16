// Helper macros that make it easy to implement a Builder child, see various Builder implementations to understand how they are used

#define AST_DECL_SUCCESSORS(BuilderClass) void AST::BuilderClass::generateSuccessors(AST::PatternID cur_ID)

#define AST_SUCC_ISTYPE(PatternType) (cur_ID == PatternID::PatternType)
#define AST_SUCC_CASE(PatternType) if (AST_SUCC_ISTYPE(PatternType))
#define AST_SUCC_ALLOW(PatternType) allowSuccessor(PatternID::PatternType)