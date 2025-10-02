Supported EBNF Grammar:
```
1.  Prog        ::= PROCEDURE ProcName IS ProcBody

2.  ProcBody    ::= DeclPart BEGIN StmtList END ProcName ;

3.  ProcName    ::= IDENT

4.  DeclPart    ::= DeclStmt { DeclStmt }

5.  DeclStmt    ::= IDENT {, IDENT } : Type [:= Expr] ;

6.  Type        ::= INTEGER | FLOAT | BOOLEAN | STRING | CHARACTER

7.  StmtList    ::= Stmt { Stmt }

8.  Stmt        ::= AssignStmt | PrintStmts | GetStmt | IfStmt

9.  PrintStmts  ::= (PutLine | Put) ( Expr ) ;

10. GetStmt     ::= Get (Var) ;

11. IfStmt      ::= IF Expr THEN StmtList { ELSIF Expr THEN StmtList } 
                    [ ELSE StmtList ] END IF ;

12. AssignStmt  ::= Var := Expr ;

13. Expr        ::= Relation { (AND | OR) Relation }

14. Relation    ::= SimpleExpr [ ( = | /= | < | <= | > | >= ) SimpleExpr ]

15. SimpleExpr  ::= STerm { ( + | - | & ) STerm }

16. STerm       ::= [ ( + | - ) ] Term

17. Term        ::= Factor { ( * | / | MOD ) Factor }

18. Factor      ::= Primary [** [(+ | -)] Primary ] | NOT Primary

19. Primary     ::= Name | ICONST | FCONST | SCONST | BCONST | CCONST | (Expr)

20. Name        ::= IDENT [ ( Range ) ]

21. Range       ::= SimpleExpr [ .. SimpleExpr ]
```
