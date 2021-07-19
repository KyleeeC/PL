# include <cstdio>
# include <cstdlib>
# include <cmath>
# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <sstream>
# include <ctype.h>

using namespace std ; 

enum TerminalType { SYMBOL = 1000, INT = 500, FLOAT = 400, STRING = 300, NIL = 200, T = 100, 
                    LEFTP = 50, RIGHTP = 40, QUOTE = 20, DOT = 10, ERROR = 5, NOTHING = 0
};

enum FunctionType { FUNC = 3000, LAMBDA = 2000, LET = 1000 } ; 


char gChar = '\0', gLastChar = '\0' ;

int gLine = 1, gColumn = 1 ;
int gNextLine = 1, gNextColumn = 1 ;
int gAbsNextLine = 1, gAbsLine = 1 ;
bool gParen = false, gSecond = false ;
bool gHasRead = true, gVerbose = true ;
int gLastLine = 0 ;
bool gExit = false ;

struct TokenS {
  int line ;
  int column ;
  string tokenStr ;     // StyleCheckType string
  double num ;
  TerminalType tType ;
};

struct Tree {
  bool exist ;
  bool protect ;
  TokenS token ;
  Tree * left ;
  Tree * right ;
  Tree * parent ;
};

typedef Tree * TreePtr ;

struct Define {
  string name ;
  TreePtr defined ;
  int num ;
  bool isEval ;
};

struct Func {
  string name ;
  int num ;
  FunctionType type ;
  vector <Define> args ;
  TreePtr defined ;
};

map <string, Define> gDefineTree ;
map <string, Define> gSystemSymbol ;
map <string, Func> gFuncDefine ;
vector <Func> gUseFunc ;
TreePtr gNoReturn ;

double Pow( int n ) {
  double rst = 1.0 ;
  if ( n >= 0 ) {
    while ( n >= 0 ) {
      rst = rst * 10 ;
      n-- ;
    } // while
  } // if
  else {
    while ( n < 0 ) {
      rst = rst / 10 ;
      n++ ;
    } // while
  } // else
  
  return rst;
} // Pow()

double My_atof( string token ) {
  bool isMinus = false, hasSym = false ;
  int pos = 0 ;
  double decimal = 0 ;
  double fraction = 0 ;
  double result ;
  if ( token.at( 0 ) == '+' || token.at( 0 ) == '-' ) {
    pos++ ;
    hasSym = true ;
    if ( token.at( 0 ) == '-' )
      isMinus = true ;
  } // if
  
  // find dot
  bool find = false ;
  while ( pos < token.size() && !find ) {
    if ( token.at( pos ) == '.' )
      find = true ;
    else
      pos++ ;
  } // while
  
  int biggest = 0, power = 0 ;
  if ( hasSym )
    biggest = 1 ;
  for ( int i = biggest ; i < pos ; i++ ) {
    if ( !isMinus )
      decimal = decimal * 10 + ( int ) ( token.at( i ) - '0' ) ;
    else
      decimal = decimal * 10 - ( int ) ( token.at( i ) - '0' ) ;
    power++ ;
  } // for
  
  power = -1 ;
  int count = 0, cur = pos+1 ;
  for ( int i = pos+1 ; i < token.size() && count < 3 ; i++ ) {
    if ( !isMinus )
      fraction =  fraction + ( int ) ( token.at( i ) - '0' ) * Pow( power ) ;
    else
      fraction = fraction - ( int ) ( token.at( i ) - '0' ) * Pow( power ) ;
    power-- ;
    count++ ;
    cur = i ;
  } // for
  
  if ( count == 3 && ( cur+1 ) < token.size() ) {
    int value = ( int ) ( token.at( cur+1 ) - '0' ) ;
    if ( value >= 5 ) {
      if ( isMinus )
        fraction = fraction - 0.001 ;
      else
        fraction = fraction + 0.001 ;
    } // if
  } // if
  
  double res = decimal + fraction ;
  return res ;
} // My_atof()

string IntToStr( int num ) {
  stringstream ss ;
  string str ;
  ss << num ;
  ss >> str ;
  return str ;
} // IntToStr()

class MyExceptionInput {
  TokenS mToken ;
  int mType ;
  int mLine, mColumn ;
public:
  MyExceptionInput( int type ) {
    mType = type ;
  } // MyExceptionInput()
  
  MyExceptionInput( TokenS token, int errorType ) {
    mToken = token ;
    mType = errorType ;
  } // MyExceptionInput()
  
  MyExceptionInput( int line, int column, int type ) {
    mLine = line ;
    mColumn = column ;
    mType = type ;
  } // MyExceptionInput()
  
  int IsEnd() {
    if ( mType == 4 )
      return true ;
    return false ;  
  } // IsEnd()
  
  void Print() {
    if ( mType == 1 ) {
      cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " ;
      cout << mLine << " Column " << mColumn << "\n\n" ;
    } // if
    else if ( mType == 2 ) {
      cout << "ERROR (unexpected token) : atom or '(' expected when token at Line " ;
      cout << mToken.line << " Column " << mToken.column << " is >>" << mToken.tokenStr << "<<\n\n" ;
    } // else if
    else if ( mType == 3 ) {
      cout << "ERROR (unexpected token) : ')' expected when token at Line " ;
      cout << mToken.line << " Column " << mToken.column << " is >>" << mToken.tokenStr << "<<\n\n" ;
    } // else if
    else if ( mType == 4 ) {
      cout << "ERROR (no more input) : END-OF-FILE encountered" ;
    } // else if
  } // Print()
  
  string Set() {
    string message = "" ;
    if ( mType == 1 ) {
      message = "\"ERROR (no closing quote) : END-OF-LINE encountered at Line " ;
      message = message + IntToStr( mLine ) + " Column " + IntToStr( mColumn ) +  "'\"" ;
    } // if
    else if ( mType == 2 ) {
      message = "\"ERROR (unexpected character) : line " + IntToStr( mToken.line )  + " column " ;
      message = message + IntToStr( mToken.column ) +" character '" + mToken.tokenStr +  "'\"" ;
    } // else if
    else if ( mType == 3 ) {
      message = "\"ERROR (unexpected character) : line " + IntToStr( mToken.line )  + " column " ;
      message = message + IntToStr( mToken.column ) + " character '" + mToken.tokenStr +  "'\"" ;
    } // else if
    else if ( mType == 4 ) {
      message = "\"ERROR : END-OF-FILE encountered when there should be more input\"" ;
    } // else if
    
    return message ;
  } // Set()
};

class Scanner { 
  bool mDoubleQ ;
  int mParen ;
  bool mFirstRead ;
  bool IsWhiteSpace( char ch ) {
    if ( ch == ' ' || ch == '\t' )
      return true ;
    else
      return false ;  
  } // IsWhiteSpace()
  
  bool SkipWhiteSpace( int & line, int & column ) {
    while ( IsWhiteSpace( gChar ) ) {
      if ( !GetChar( line, column ) )
        return false ;
    } // while()
    
    return true ;
  } // SkipWhiteSpace()  
  
  bool IsSeparator( char ch ) {
    if ( IsWhiteSpace( gChar ) || gChar == '\n' )
      return true ;
    else if ( gChar == '(' || gChar == ')' || gChar == ';' )
      return true ;
    else if ( gChar == '\'' )
      return true ;
    else  
      return false ;
           
  } // IsSeparator()
  
  bool IsNum( char ch ) {
    if ( ch >= '0' && ch <= '9' )
      return true ;
    else
      return false ;  
  } // IsNum()
  
  bool IsInt( string token ) {
    if ( token == "+" || token == "-" || token.length() == 0 )
      return false ;
    for ( int i = 0 ; i < token.length() ; i++ ) {
      if ( i == 0 ) {
        if ( token.at( i ) == '+' || token.at( i ) == '-' || IsNum( token.at( i ) ) ) ;
        else
          return false ;      
      } // if
      else {
        if ( !IsNum( token.at( i ) ) )
          return false ;
      } // else
    } // for
    
    if ( token.at( 0 ) == '+' )
      token.erase( token.begin() ) ; 

    return true ;
  } // IsInt() 
  
  bool IsFloat( string token ) {
      // DOT¤w¸g¦b¤§«e§PÂ_§¹ 
    bool findDot = false, findNum = false ;
    int pos = -1 ;
    for ( int i = 0 ; i < token.length() ; i++ ) {
      if ( i == 0 ) {
        if ( token.at( i ) == '+' || token.at( i ) == '-' || IsNum( token.at( i ) ) ) ;
        else if ( token.at( i ) == '.' && !findDot ) {
          pos = i ;
          findDot = true ;
        } // else if
        else
          return false ;      
      } // if
      else {
        if ( IsNum( token.at( i ) ) ) ;
        else if ( token.at( i ) == '.' && !findDot ) {
          pos = i ;
          findDot = true ;
        } // else if
        else  
          return false ;
      } // else
      
      if ( IsNum( token.at( i ) ) )
        findNum = true ;
    } // for
    
    if ( findDot && findNum )       
      return true ;     
    return false ;     
  } // IsFloat()
  
  bool IsString( string token ) {
    if ( token.at( 0 ) == '\"' && token.at( token.length()-1 ) == '\"' )
      return true ;
    else
      return false ;  
  } // IsString()
  
  string FloatNum( string token ) {
    double temp = My_atof( token ) ;
    if ( temp != ( int ) temp ) {
      temp = temp * 1000 ;
      temp = round( temp ) ;
      temp = temp / 1000 ;
      stringstream sstream;

      sstream << temp ;
      token = sstream.str() ;
    } // if
    
    if ( token.at( 0 ) == '+' )
      token.erase( token.begin() ) ; 
    
    bool find = false ;
    string final = "" ;
    int pos = 0 ;
    while ( !find ) {
      final = final + token.at( pos ) ;
      if ( token.at( pos ) == '.' ) {
        find = true ;
        if ( pos == 0 )
          final = "0" + final ;
      } // if
      
      pos++ ;
    } // while
    
    int count = 0 ;
    int i = pos ;
    for ( ; i < token.length() && count < 3 ; i++ ) {
      count++ ;
      final = final + token.at( i ) ;
    } // for
    
    int num = token.length() - pos ;
    for ( ; num < 3 ; num++ ) {
      final = final + "0" ;
    } // for
    
    return final ;

    
  } // FloatNum()
  
  bool MakeToken( TokenS & tkn, bool & com, 
                  int & line, int & column ) {
    mDoubleQ = false ;
    int lineSP = line, columnSP = column ;
    while ( IsWhiteSpace( gChar ) ) {
      if ( !SkipWhiteSpace( lineSP, columnSP ) )
        return false ;
    } // while()
    
    TokenS temp ;
    string tempS = "", backup = "" ;
    bool special = false ;   // special : string continue read
    bool change = false, jump = false ;
    bool notEOF = true ;
    while ( ( special || !IsSeparator( gChar ) ) && !jump && notEOF ) {
      backup = tempS ;
      tempS = tempS + gChar ;   
      if ( gChar == '\"' && !special ) {
        if ( backup == "" ) {
          special = true ;
          tempS = "\"" ;
        } // if
        else {
          mDoubleQ = true ;   // jump and dont't read
          tempS = backup ;
          jump = true ;
        } // else
      } // if
      else if ( gChar == '\"' && special ) {
        special = false ;
        jump = true ;
        mDoubleQ = false ;
      } // else if
      
      if ( gChar == '\\' && special ) {
        if ( !GetChar( line, column ) )
          notEOF = false ;
          
        if ( gChar == 'n' )
          tempS = backup + '\n' ;
        else if ( gChar == '\"' )
          tempS = backup + '\"' ;
        else if ( gChar == 't' )
          tempS = backup + '\t' ;
        else if ( gChar == '\'' )
          tempS = backup + '\'' ; 
        else if ( gChar == '\\' )
          tempS = backup + '\\' ;  
        else
          tempS = tempS + gChar ; 
      } // if
      
      if ( !jump )
        if ( !GetChar( line, column ) )
          notEOF = false ;  
          
      if ( gChar == '\n' && special ) {
        special = false ; 
        throw MyExceptionInput( gLine, gColumn, 1 ) ;
      } // if
       
    }  // while
    
    // -----------------------------------finish get token--------------------

    if ( gChar == ';' ) {
      bool isRead = mFirstRead ;
      do {    
        if ( !GetChar( line, column ) )
          notEOF = false ;
            
        if ( gChar == '\n' )
          change = true ; 
        
        mFirstRead = isRead ;   
      } while ( !change && notEOF ) ;
      
      mFirstRead = false ;
    } // if
    
    temp.tokenStr = tempS ;
    if ( gChar == ';' && tempS == "" )
      com = true ;
    else
      com = false ;  
    
    if ( !notEOF && tempS == "" )
      throw MyExceptionInput( 4 ) ; 
    else if ( temp.tokenStr == "" ) ;
    else if ( IsInt( tempS ) ) {
      int tempI = atoi( temp.tokenStr.c_str() ) ;
      stringstream sstream;
      sstream << tempI ;
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = sstream.str() ;
      temp.tType = INT ;
    } // if
    else if ( IsString( tempS ) ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = STRING ;
    } // else if
    else if ( tempS == "." ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = DOT ;
    } // else if
    else if ( IsFloat( tempS ) ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = FloatNum( tempS ) ; 
      temp.tType = FLOAT ;
      temp.num = My_atof( tempS ) ;
    } // else if
    else if ( tempS == "nil" || tempS == "#f" ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = "nil" ;
      temp.tType = NIL ;
    } // else if
    else if ( tempS == "t" || tempS == "#t" ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = "#t" ;
      temp.tType = T ;
    } // else if

    else {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = SYMBOL ;
    } // else

    gSecond = false ;
    if ( gChar == '(' ) {
      if ( temp.tokenStr != "" ) {
        gSecond = true ;
      } // if
      else {
        mParen++ ;
        temp.line = gLine ;
        temp.column = gColumn ;
        temp.tokenStr = "(" ;
        temp.tType = LEFTP ;
      } // else
    }  // if
    else if ( gChar == ')' ) {
      if ( temp.tokenStr != "" ) {
        gSecond = true ;
      } // if
      else {
        mParen-- ;
        temp.line = gLine ;
        temp.column = gColumn ;
        temp.tokenStr = ")" ;
        temp.tType = RIGHTP ;
        tkn = temp ;
      } // else
    } // else if
    else if ( gChar == '\'' ) {
      if ( !gSecond && temp.tokenStr != "" ) {
        gSecond = true ;
      } // if
      else {
        temp.line = gLine ;
        temp.column = gColumn ;
        temp.tokenStr = "\'" ;
        temp.tType = QUOTE ;
      } // else if
    } // else if
    
    tkn = temp ;
    if ( tkn.tokenStr.size() > 0 )
      return true ;
    return false ;
  } // MakeToken()
  
  bool GetToken( TokenS & tkn ) {
    bool skip_com = false, make = true ;
    bool quote = false ;
    bool jump = false, isEnd = false ;  
    int line, column ;
    do {
      if ( !gSecond && !mDoubleQ ) {
        if ( !GetChar( line, column ) )
          throw MyExceptionInput( 4 ) ;
      } // if

      make = MakeToken( tkn, skip_com, line, column ) ;    
      if ( skip_com )
        Skip() ;
    } while ( !make ) ;
    
    return true ;
  } // GetToken() 
  
  bool Atom( TokenS tkn, vector <TokenS> & expr ) {
    if ( tkn.tType == SYMBOL ) {
      expr.push_back( tkn ) ;
      return true ;
    } // if
    else if ( tkn.tType == INT ) {
      expr.push_back( tkn ) ;
      return true ;
    } // if
    else if ( tkn.tType == FLOAT ) {
      expr.push_back( tkn ) ;
      return true ;
    } // if
    else if ( tkn.tType == STRING ) {
      expr.push_back( tkn ) ;
      return true ;
    } // if
    else if ( tkn.tType == NIL ) {
      expr.push_back( tkn ) ;
      return true ;
    } // if
    else if ( tkn.tType == T ) {
      expr.push_back( tkn ) ;
      return true ;
    } // if
    else if ( tkn.tType == LEFTP ) {
      char ch = PeekToken() ;
      if ( ch != ')' )
        return false ;
      expr.push_back( tkn ) ;
      GetToken( tkn ) ;
      expr.push_back( tkn ) ;
      return true ;
    } // else if
    else
      return false ;               
  } // Atom()
 
public: 
  Scanner() {
    mFirstRead = true ;
  } // Scanner()

  bool SExp( vector <TokenS> & expr, bool get, TokenS tkn ) {
    if ( get )
      GetToken( tkn ) ;      
    if ( Atom( tkn, expr ) )
      return true ;
    else if ( tkn.tokenStr == "(" ) {
      expr.push_back( tkn ) ;
      SExp( expr, true, tkn ) ;
      GetToken( tkn ) ;
      while ( tkn.tType != DOT && tkn.tokenStr != ")" ) {
        SExp( expr, false, tkn ) ;
        GetToken( tkn ) ;
      } // while
      
      if ( tkn.tType == DOT ) {
        expr.push_back( tkn ) ; 
        SExp( expr, true, tkn ) ;
        GetToken( tkn ) ;
      } // if

      if ( tkn.tokenStr == ")" ) {
        expr.push_back( tkn ) ;
        return true ;
      } // if
      else {
        throw MyExceptionInput( tkn, 3 ) ;
      } // else
    } // else if
    else if ( tkn.tokenStr == "\'" ) {
      expr.push_back( tkn ) ;
      SExp( expr, true, tkn ) ;
      return true ; 
    } // else if
    else {
      throw MyExceptionInput( tkn, 2 ) ;
    } // else  
    
    return false ;    
  } // SExp()


  void Skip() {
    bool change = false ;
    int line, column ;
    if ( gChar == '\n' )
      change = true ; 
      
    while ( !change ) {
      if ( !GetChar( line, column ) )
        change = true ;
            
      if ( gChar == '\n' )
        change = true ; 
    } // while
    
    gNextLine = 1 ;
    gNextColumn = 1 ;
  } // Skip()

  bool GetChar( int & line, int & column ) {
    gLastChar = gChar ;
    if ( scanf( "%c", &gChar ) != EOF ) {
      line = gNextLine ;
      column = gNextColumn ;
      gLine = gNextLine ;
      gColumn = gNextColumn ;
      gAbsLine = gAbsNextLine ;
      if ( gChar != '\n' ) {
        gNextColumn++ ;
      } // if
      else {
        if ( gAbsLine == gLastLine && mFirstRead ) {
          gLine = 1 ;
          gNextLine = 1 ; 
          gAbsNextLine = 1;
          gNextColumn = 1 ;
        } // if
        else {
          gAbsNextLine++ ;
          gNextLine++ ;
          gNextColumn = 1 ; 
        } // else
        
      } // else
      
      if ( gChar != ' ' && gChar != '\t' && gChar != ';' )
        mFirstRead = false ;
      return true ;
    } // if
    else
      return false ;  
      
  } // GetChar()
  
  char PeekToken() {
    char ch = cin.peek() ;
    while ( IsWhiteSpace( ch ) || ch == '\n' ) {
      int line, column ;
      if ( !GetChar( line, column ) )
        throw MyExceptionInput( 4 ) ;
      
      ch = cin.peek() ;
    } // while
    
    return ch ;
  } // PeekToken()
   
  bool JumpChar( char ch ) {
    if ( gChar == '\t' || gChar == ' ' || gChar == '\n' )
      return true ;
    else if ( gChar == ')' || gChar == ';' || gChar == '\"' )
      return true ;
    else  
      return false ;
  } // JumpChar() 
  
  void Paren( vector <TokenS> & expr ) {
    for ( int i = 0 ; i < expr.size() ; i++ ) {
      if ( expr.at( i ).tType == LEFTP )
        if ( i+1 < expr.size() && expr.at( i+1 ).tType == RIGHTP ) {
          expr.erase( expr.begin()+i+1 ) ;
          expr.at( i ).tokenStr = "nil" ;
          expr.at( i ).tType = NIL ;
        } // if
    } // for
  } // Paren()
};

class MakeTree {
  int mlevel ;
  TreePtr mhead ;
  bool mspace ;
  int mspaceNum ;  
public:
  MakeTree() {
    mhead = new Tree ;
    mhead->exist = false ;
    mhead->left = NULL ;
    mhead->right = NULL ;
    mhead->parent = NULL ;
    mlevel = 0 ;
    mspace = false ;
  } // MakeTree()
  
  void Delete( TreePtr & tree ) {
    if ( tree != NULL && !tree->protect ) {
      if ( tree->left == NULL && tree->right == NULL ) {
        delete tree ;
        tree = NULL ;
      } // if
      else {
        Delete( tree->left ) ;
        tree->left = NULL ;
        Delete( tree->right ) ;
        tree->right = NULL ;
        delete tree ;
        tree = NULL ;
      } // else
    } // if
  } // Delete()
  
  
  
  
  bool IsAtomNotNIL( TerminalType type ) {
    if ( type == INT )
      return true ;
    else if ( type == FLOAT )
      return true ;
    else if ( type == STRING )
      return true ;
    else if ( type == T )
      return true ;
    else if ( type == SYMBOL )
      return true ;
    else          
      return false ;
  } // IsAtomNotNIL()
  
  bool IsAtom( TerminalType type ) {
    if ( type == NIL )
      return true ;
    else if ( IsAtomNotNIL( type ) )
      return true ;
    else if ( type == ERROR )
      return true ;
    return false ;
  } // IsAtom()
  
  void Print( TreePtr tree, int spaceNum ) {
    if ( tree == NULL ) ;
    else if ( IsAtom( tree->token.tType ) ) {
      if ( mspace )
        mspace = false ;
      else
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
        
      if ( tree->token.tType != FLOAT )
        cout << tree->token.tokenStr << "\n" ;
      else
        printf( "%.3f\n", tree->token.num ) ;
    } // else if
    else {
      if ( !mspace ) {
        for ( int i = 0 ; i < spaceNum ; i++ ) {
          cout << "  " ;
        } // for
      } // if
        
      cout << "( " ;
      mspace = true ;   
      TreePtr walk = tree ;
      bool finish = false ;
      while ( walk != NULL && !finish ) {
        Print( walk->left, spaceNum+1 ) ; 
        walk = walk->right ;
        if ( walk->token.tType == NIL )
          finish = true ;
        else if ( IsAtomNotNIL( walk->token.tType ) && walk->right == NULL ) {
          for ( int i = 0 ; i < spaceNum+1 ; i++ )
            cout << "  " ;          
          cout << "." << "\n" ;
          if ( mspace )
            mspace = false ;
          else
            for ( int i = 0 ; i < spaceNum+1 ; i++ )
              cout << "  " ;            
          if ( walk->token.tType != FLOAT )
            cout << walk->token.tokenStr << "\n" ;
          else
            printf( "%.3f\n", walk->token.num ) ;              
          finish = true ;
        } // if
      } // while
      
      for ( int i = 0 ; i < spaceNum ; i++ )
        cout << "  " ;
      cout << ")\n" ;
      mspace = false ; 
    } // else
  } // Print()
  
  void Create( vector <TokenS> expr ) {
    TreePtr parent = mhead ;
    TreePtr temp = mhead ;
    for ( int i = 0 ; i < expr.size() ; i++ ) {
      /*
      Print( mhead, 0 ) ;
      cout << "Current : " << expr.at( i ).tokenStr << "\n" ;
      for ( int j = i ; j < expr.size() ; j++ ) {
        cout << expr.at( j ).tokenStr << " " ;
      }
      cout << "\nWalk : \n";
      for ( TreePtr walk = temp ; walk != NULL ; walk = walk->parent ) {
        cout << "[" << walk->token.tokenStr << "]" << "\n" ;
      }
      cout << "\n============\n" ;
      */
      
      if ( expr.at( i ).tType == LEFTP ) {
        if ( parent->right != NULL && temp == parent->right ) {
          temp->exist = false ;
          temp->token.tType = NOTHING ;
          temp->left = new Tree ;
          temp->left->protect = false ;
          temp->left->token.tType = NOTHING ;
          temp->right = NULL ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
        } // if
        
        mlevel++ ;
        temp->exist = true ;
        temp->token = expr.at( i ) ;
        temp->left = new Tree ;
        temp->left->protect = false ;
        temp->left->token.tType = NOTHING ;
        temp->right = NULL ;
        parent = temp ;
        temp->left->parent = parent ;
        temp = temp->left ;
        temp->left = NULL ;
        temp->right = NULL ; 
      } // if
      else if ( expr.at( i ).tType == RIGHTP ) {
        bool find = false ;
        while ( !find && temp != NULL ) {
          if ( temp->exist
               && ( temp->token.tType == LEFTP || temp->token.tType == QUOTE ) ) {
            find = true ;
            if ( temp != mhead ) {
              temp = temp->parent ;
              parent = temp->parent ;
            } // if
          } // if
          else {
            if ( temp != mhead ) {
              temp = temp->parent ;
              parent = temp->parent ;
            } // if
          } // else 
          
        } // while
        
        mlevel-- ;
        if ( i == expr.size()-1 ) ;
        else if ( i < expr.size()-1 && expr.at( i+1 ).tType == DOT ) ;
        else if ( i < expr.size()-1 && ( expr.at( i+1 ).tType == LEFTP || expr.at( i+1 ).tType == QUOTE ) ) {
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else if
        else if ( i < expr.size()-1 && expr.at( i+1 ).tType == RIGHTP ) {
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->token.tType = NIL ;
          temp->token.tokenStr = "nil" ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else if
        else {    // ATOM
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->left = new Tree ;
          temp->left->protect = false ;
          temp->left->token.tType = NOTHING ;
          temp->right = NULL ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
          temp->exist = false ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else
        
      } // else if
      else if ( expr.at( i ).tType == QUOTE ) {
        if ( parent->right != NULL && temp == parent->right ) {
          temp->exist = false ;
          temp->left = new Tree ;
          temp->left->protect = false ;
          temp->left->token.tType = NOTHING ;
          temp->right = NULL ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
        } // if
  
        mlevel++ ;
        expr.at( i ).tokenStr = "(" ;
        expr.at( i ).tType = LEFTP ;
        temp->exist = true ;
        temp->token = expr.at( i ) ;
        temp->left = new Tree ;
        temp->left->protect = false ;
        temp->left->token.tType = NOTHING ;
        temp->left->parent = temp ;
        temp->left->exist = true ;
        temp->left->token.tokenStr = "quote" ;
        temp->left->token.tType = SYMBOL ;
        temp->left->left = NULL ;
        temp->left->right = NULL ;
        temp->right = new Tree ;
        temp->right->protect = false ;
        temp->right->token.tType = NOTHING ;
        parent = temp ;
        temp->right->parent = parent ;
        temp = temp->right ;
        temp->exist = false ;
        temp->left = NULL ;
        temp->right = NULL ;
        TokenS tempS ;
        tempS.tokenStr = ")" ;
        tempS.tType = RIGHTP ;
        bool find = false ;
        int pos = i+1 ;
        for ( ; pos < expr.size() && !find ; pos++ ) {
          if ( expr.at( pos ).tType == QUOTE ) ;
          else {
            find = true ;
            pos-- ;
          } // else
        } // for
        
        if ( i < expr.size()-1 && expr.at( pos ).tType == LEFTP ) {  
          int paren = 0 ;
          bool jump = false ;
          for ( int j = i+1 ; j < expr.size() && !jump ; j++ ) {
            if ( expr.at( j ).tType == LEFTP )
              paren++ ;
            else if ( expr.at( j ).tType == RIGHTP ) {
              paren-- ;
              if ( paren == 0 ) {
                expr.insert( expr.begin()+j+1, tempS ) ;
                jump = true ;
              } // if
            } // else if   
          } // for
          
          if ( !jump ) {
            expr.push_back( tempS ) ;
            jump = true ;
          } // if
        } // if
        else {
          expr.insert( expr.begin()+pos+1, tempS ) ;
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          temp->right->exist = false ;
          temp->right->parent = temp ;
          temp->right->token.tokenStr = "nil" ;
          temp->right->token.tType = NIL ;
          temp->right->left = NULL ;
          temp->right->right = NULL ;
          temp->left = new Tree ;
          temp->left->protect = false ;
          temp->left->token.tType = NOTHING ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else
      } // else if
      else if ( expr.at( i ).tType == DOT ) {
        if ( expr.at( i+1 ).tType == LEFTP ) {
          temp->right = new Tree ;  
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;       
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->left = NULL ;
          temp->right = NULL ; 
          expr.erase( expr.begin() + i ) ;   // erase DOT
          expr.erase( expr.begin() + i ) ;   // erase LEFTP
          bool find = false ;
          int count = 0 ;
          for ( int j = i ; !find && j < expr.size() ; j++ ) {
            if ( expr.at( j ).tType == RIGHTP ) {
              if ( count > 0 )
                count-- ;
              else {
                find = true ;
                expr.erase( expr.begin() + j ) ;
              } // else
            } // if
            else if ( expr.at( j ).tType == LEFTP ) 
              count++ ;
              
          } // for
  
          i = i-1 ;
          if ( expr.at( i+1 ).tType != LEFTP && expr.at( i+1 ).tType != QUOTE ) {
            temp->left = new Tree ;
            temp->left->protect = false ;
            temp->left->token.tType = NOTHING ;
            parent = temp ;
            temp->left->parent = parent ;
            temp = temp->left ;
            temp->left = NULL ;
            temp->right = NULL ;
          }  // if
        } // if
        else if ( expr.at( i+1 ).tType == NIL ) {
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->token = expr.at( i+1 ) ;
          temp->left = NULL ;
          temp->right = NULL ;
          
          expr.erase( expr.begin() + i ) ;   // erase DOT
          expr.erase( expr.begin() + i ) ;   // erase NIL
          i = i - 1 ;
        } // else if
        else {    // Is ATOM
          temp->exist = true ;
          if ( temp->token.tokenStr != "(" )
            temp->token = expr.at( i ) ;
            
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else
      } // if
      else { // ATOM
        temp->exist = true ;
        temp->token = expr.at( i ) ;
        if ( temp->parent != NULL ) {
          temp = temp->parent ;
          parent = parent->parent ;
        } // if 

        if ( i < expr.size()-1 && expr.at( i+1 ).tType == RIGHTP ) {
          if ( temp->right == NULL ) {
            temp->right = new Tree ;
            temp->right->protect = false ;
            temp->right->token.tType = NOTHING ;
            parent = temp ;
            temp->right->parent = parent ;
            temp = temp->right ;
            temp->exist = false ;
            temp->token.tType = NIL ;
            temp->token.tokenStr = "nil" ;
            temp->left = NULL ;
            temp->right = NULL ; 
          } // if
          else {
            parent = temp ;
            temp = temp->right ;
          } // else
        } // if
        else if ( i < expr.size()-1 && ( expr.at( i+1 ).tType == LEFTP || expr.at( i+1 ).tType == QUOTE ) ) {
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->left = NULL ;
          temp->right = NULL ; 
        } // else if
        else if ( i < expr.size()-1 && expr.at( i+1 ).tType == DOT ) ;
        else if ( i < expr.size()-1 ) {
          temp->right = new Tree ;
          temp->right->protect = false ;
          temp->right->token.tType = NOTHING ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->right = NULL ; 
          temp->left = new Tree ;
          temp->left->protect = false ;
          temp->left->token.tType = NOTHING ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else if
      } // else
    } // for
    
  } // Create()
    
  void Print() {
    TreePtr temp = mhead ;
    mspaceNum = 0 ;
    Print( temp, 0 ) ;
  } // Print()
  
  void Clear() {
    mhead = NULL ;
    // Delete( temp ) ;
  } // Clear()
  
  void Build() {
    mhead = new Tree ;
    mhead->protect = false ;
    mhead->token.tType = NOTHING ;
    mhead->exist = false ;
    mhead->left = NULL ;
    mhead->right = NULL ;
    mhead->parent = NULL ;
    mlevel = 0 ;
  } // Build()
  
  TreePtr GetTree() {
    return mhead ;
  } // GetTree()
}; // MakeTree

bool ReadSExp( vector <TokenS> & expr, bool & end,
               MakeTree & makeTree, bool read, string & message ) ;

class MyException {
  string mtoken ;
  TreePtr mhead ;
  int mtype ;
  MakeTree mtree ;
  
  string LowerToUpper( string lower ) {
    string upper = "" ;
    for ( int i = 0 ; i < lower.size() ; i++ ) {
      char ch = toupper( lower.c_str()[i] ) ;
      upper = upper + ch ;
    } // for
    
    return upper ;
  } // LowerToUpper()
  
public:
  
  MyException( string str, TreePtr cur, int errorType ) {
    mtoken = str ;
    mhead = cur ;
    mtype = errorType ;
  } // MyException()
  
  void Error() {
    if ( mtype == 1 ) {
      cout << "ERROR (unbound symbol) : " << mtoken << "\n\n" ;
    } // if
    else if ( mtype == 2 ) {
      cout << "ERROR (non-list) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 3 ) {
      cout << "ERROR (attempt to apply non-function) : " << mtoken << "\n\n" ;
    } // else if
    else if ( mtype == 4 ) {
      cout << "ERROR (level of " << LowerToUpper( mtoken ) << ")\n\n" ;
    } // else if
    else if ( mtype == 5 ) {
      cout << "ERROR (incorrect number of arguments) : " << mtoken << "\n\n" ;
    } // else if
    else if ( mtype == 6 ) {
      cout << "ERROR (" << LowerToUpper( mtoken ) << " format) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 7 ) {
      cout << "ERROR (attempt to apply non-function) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 8 ) {
      cout << "ERROR (no return value) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 9 ) {
      cout << "ERROR (unbound parameter) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 10 ) {
      cout << "ERROR (" << mtoken << " with incorrect argument type) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 11 ) {
      cout << "ERROR (unbound test-condition) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 12 ) {
      cout << "ERROR (unbound condition) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 13 ) {
      cout << "ERROR (division by zero) : /\n\n" ;
    } // else if
  } // Error()
};

class MakeIns {
  bool mspace ;
public:
  MakeIns() {
    mspace = false ;
  } // MakeIns()
  
  bool IsAtomNotNIL( TerminalType type ) {
    if ( type == INT )
      return true ;
    else if ( type == FLOAT )
      return true ;
    else if ( type == STRING )
      return true ;
    else if ( type == T )
      return true ;
    else if ( type == SYMBOL )
      return true ;
    else          
      return false ;
  } // IsAtomNotNIL()
  
  
  bool IsAtomNotSym( TerminalType type ) {
    if ( type == INT )
      return true ;
    else if ( type == FLOAT )
      return true ;
    else if ( type == STRING )
      return true ;
    else if ( type == NIL )
      return true ;
    else if ( type == T )
      return true ;
    else          
      return false ;
  } // IsAtomNotSym()
  
  string ChangeType( string token ) {
    if ( token.size() < 14 )
      return token ;
      
    string temp = token ;
    temp.erase( temp.begin()+temp.size()-1 ) ;
    temp.erase( 0, 12 ) ;
    map<string,Define>::iterator get = gSystemSymbol.find( temp ) ;
    if ( get != gSystemSymbol.end() )
      return temp ;
    else {
      map<string,Func>::iterator i = gFuncDefine.find( temp ) ;
      if ( i != gFuncDefine.end() )
        return temp ;
      else
        return token ;
    } // else
  } // ChangeType()
  
  void MakeSystemDefine( string token, int num ) {
    Define temp ;
    temp.name = token ;
    temp.defined = new Tree ;
    temp.defined->protect = false ;
    temp.defined->exist = true ;
    temp.defined->left = NULL ;
    temp.defined->right = NULL ;
    temp.defined->parent = NULL ;
    temp.defined->token.tokenStr = "#<procedure " + token + ">" ;
    temp.defined->token.tType = STRING ;
    temp.num = num ;
    gSystemSymbol[token] = temp ;
  } // MakeSystemDefine()
  
  void MakeFuncDefine( string token ) {
    Define temp ;
    temp.name = token ;
    temp.defined = new Tree ;
    temp.defined->protect = false ;
    temp.defined->exist = true ;
    temp.defined->left = NULL ;
    temp.defined->right = NULL ;
    temp.defined->parent = NULL ;
    temp.defined->token.tokenStr = "#<procedure " + token + ">" ;
    temp.defined->token.tType = STRING ;
    gDefineTree[token] = temp ;
  } // MakeFuncDefine()
  
  void Set() {
    MakeSystemDefine( "cons", 2 ) ;
    MakeSystemDefine( "list", 4 ) ;
    MakeSystemDefine( "quote", 1 ) ;
    MakeSystemDefine( "define", 2 ) ;
    MakeSystemDefine( "car", 1 ) ;
    MakeSystemDefine( "cdr", 1 ) ;
    MakeSystemDefine( "atom?", 1 ) ;
    MakeSystemDefine( "pair?", 1 ) ;
    MakeSystemDefine( "list?", 1 ) ;
    MakeSystemDefine( "null?", 1 ) ;
    MakeSystemDefine( "integer?", 1 ) ;
    MakeSystemDefine( "real?", 1 ) ;
    MakeSystemDefine( "number?", 1 ) ;
    MakeSystemDefine( "string?", 1 ) ;
    MakeSystemDefine( "boolean?", 1 ) ;
    MakeSystemDefine( "symbol?", 1 ) ;
    MakeSystemDefine( "+", 6 ) ;
    MakeSystemDefine( "-", 6 ) ;
    MakeSystemDefine( "*", 6 ) ;
    MakeSystemDefine( "/", 6 ) ;
    MakeSystemDefine( "not", 1 ) ;
    MakeSystemDefine( "and", 6 ) ;
    MakeSystemDefine( "or", 6 ) ;
    MakeSystemDefine( ">", 6 ) ;
    MakeSystemDefine( ">=", 6 ) ;
    MakeSystemDefine( "<", 6 ) ;
    MakeSystemDefine( "<=", 6 ) ;
    MakeSystemDefine( "=", 6 ) ;
    MakeSystemDefine( "string-append", 6 ) ;
    MakeSystemDefine( "string>?", 6 ) ;
    MakeSystemDefine( "string<?", 6 ) ;
    MakeSystemDefine( "string=?", 6 ) ;
    MakeSystemDefine( "eqv?", 2 ) ;
    MakeSystemDefine( "equal?", 2 ) ;
    MakeSystemDefine( "begin", 5 ) ;
    MakeSystemDefine( "if", -1 ) ;
    MakeSystemDefine( "cond", 5 ) ;
    MakeSystemDefine( "clean-environment", 0 ) ;
    MakeSystemDefine( "exit", 0 ) ;
    MakeSystemDefine( "lambda", 6 ) ;
    MakeSystemDefine( "let", 6 ) ;
    MakeSystemDefine( "set!", 2 ) ;
    MakeSystemDefine( "verbose?", 0 ) ;
    MakeSystemDefine( "verbose", 1 ) ;
    MakeSystemDefine( "error-object?", 1 ) ;
    MakeSystemDefine( "create-error-object", 1 ) ;
    MakeSystemDefine( "read", 0 ) ;
    MakeSystemDefine( "write", 1 ) ;
    MakeSystemDefine( "display-string", 1 ) ;
    MakeSystemDefine( "newline", 0 ) ;
    MakeSystemDefine( "symbol->string", 1 ) ;
    MakeSystemDefine( "number->string", 1 ) ;
    MakeSystemDefine( "eval", 1 ) ;
  } // Set()
  
  bool CheckSymbol( TreePtr cur ) {
    TreePtr temp = cur ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) {
        if ( temp->left != NULL && temp->left->token.tType != SYMBOL )
          return false ;
      } // if
      
      temp = temp->right ;
    } // while
    
    return true ;
  } // CheckSymbol()
  
  int GetArgsNum( TreePtr cur ) {
    int num = 0 ;
    TreePtr temp = cur ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) 
        num++ ;
      temp = temp->right ;
    } // while
    
    return num ;
  } // GetArgsNum()
  
  bool CheckArgsEqual( TreePtr cur, int num ) {
    int count = 0 ;
    TreePtr temp = cur ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) {
        count++ ;
        if ( count > num )
          return false ;
      } // if

      temp = temp->right ;
    } // while
    
    if ( count == num )
      return true ;
    return false ;
  } // CheckArgsEqual()
  
  bool CheckArgsBigger( TreePtr cur, int num ) {
    int count = 0 ;
    TreePtr temp = cur ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) {
        count++ ;
        if ( count >= num )
          return true ;
      } // if

      temp = temp->right ;
    } // while
    
    if ( count >= num )
      return true ;
    return false ;
  } // CheckArgsBigger()
  
  bool CheckArgsNum( TreePtr cur, string primitive ) {  
    // check args is correct
    map<string,Define>::iterator find = gSystemSymbol.find( primitive ) ;
    if ( find != gSystemSymbol.end() ) {
      if ( find->second.num == -1 ) {
        int num = GetArgsNum( cur->right ) ;
        if ( num == 2 || num == 3 )
          return true ;
        else
          return false ;
      } // if
      else if ( find->second.num >= 0 && find->second.num <= 2 ) {
        return CheckArgsEqual( cur->right, find->second.num ) ;
      } // else if
      else {
        return CheckArgsBigger( cur->right, find->second.num-4 ) ;
      } // else
    } // if
    else {
      map<string,Func>::iterator result = gFuncDefine.find( primitive ) ;
      if ( result != gFuncDefine.end() ) {
        if ( CheckArgsEqual( cur->right, result->second.num ) )
          return true ;
        else
          return false ;
      } // if
      
      return false ; 
    } // else
  } // CheckArgsNum()
  
  void Print( TreePtr tree, int spaceNum, bool change ) {
    if ( tree == NULL ) ;
    else if ( IsAtom( tree->token.tType ) || tree->token.tType == ERROR ) {
      if ( mspace )
        mspace = false ;
      else
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
        
      if ( tree->token.tType != FLOAT )
        cout << tree->token.tokenStr ;
      else
        printf( "%.3f", tree->token.num ) ;
        
      if ( !change || spaceNum != 0 )
        cout << "\n" ;
    } // else if
    else {
      if ( !mspace ) {
        for ( int i = 0 ; i < spaceNum ; i++ ) {
          cout << "  " ;
        } // for
      } // if
        
      cout << "( " ;
      mspace = true ;   
      Print( tree->left, spaceNum+1, change ) ; 
      TreePtr walk = tree->right ;
      bool finish = false ;
      while ( walk != NULL && !finish ) {
        if ( walk != NULL && walk->token.tType == NIL )
          finish = true ;
        else if ( walk != NULL && ( IsAtom( walk->token.tType ) 
                                    || tree->token.tType == ERROR ) ) {
          for ( int i = 0 ; i < spaceNum+1 ; i++ )
            cout << "  " ;
          
          cout << "." << "\n" ;
          if ( mspace )
            mspace = false ;
          else
            for ( int i = 0 ; i < spaceNum+1 ; i++ )
              cout << "  " ;
            
          if ( walk->token.tType != FLOAT )
            cout << walk->token.tokenStr << "\n" ;
          else
            printf( "%.3f\n", walk->token.num ) ;
              
          finish = true ;
        } // if
        else
          Print( walk->left, spaceNum+1, change ) ; 
        
        walk = walk->right ;
      } // while
      
      for ( int i = 0 ; i < spaceNum ; i++ )
        cout << "  " ;
      cout << ")" ;
      if ( !change || spaceNum != 0 )
        cout << "\n" ;
      mspace = false ; 
    } // else
  } // Print()

  void Find( TreePtr tree, string & result ) {
    if ( tree == NULL ) ;
    else if ( IsAtom( tree->token.tType ) || tree->token.tType == ERROR )
      result = result + tree->token.tokenStr ;
    else {
      result = result + "(" ;
      TreePtr walk = tree ;
      bool finish = false ;
      while ( walk != NULL && !finish ) {
        Find( walk->left, result ) ; 
        walk = walk->right ;
        if ( walk->token.tType == NIL )
          finish = true ;
        else if ( ( IsAtom( walk->token.tType ) || walk->token.tType == ERROR )
                  && walk->right == NULL ) {
          result = result + "." + walk->token.tokenStr ;
          finish = true ;
        } // if
      } // while

      result  = result + ")" ; 
    } // else
  } // Find()

  bool IsPrimitive( string name ) {
    map<string,Define>::iterator result = gSystemSymbol.find( name ) ;
    if ( result != gSystemSymbol.end() )
      return true ; 
    else
      return false ;
  } // IsPrimitive()
  
  bool IsKnown( string name ) {
    if ( IsPrimitive( name ) )
      return true ;
    else {
      map<string,Define>::iterator result = gDefineTree.find( name ) ;
      if ( result != gDefineTree.end() ) {
        string token = ChangeType( result->second.defined->token.tokenStr ) ;
        if ( token != result->second.defined->token.tokenStr )
          return true ;
      } // if
      
      /*
      for ( int i = 0 ; i < gDefineTree.size() ; i++ ) {
        if ( name == gDefineTree.at( i ).name ) {
          string token = ChangeType( gDefineTree.at( i ).defined->token.tokenStr ) ;
          if ( token != gDefineTree.at( i ).defined->token.tokenStr )
            return true ;
        } // if
      } // for 
      */
      
      return false ;
    } // else
  } // IsKnown()
  
  bool PureList( TreePtr head ) {
    TreePtr walk = head ;
    while ( walk != NULL ) {
      if ( walk->right != NULL && IsAtomNotNIL( walk->right->token.tType )  )
        return false ;
      walk = walk->right ;
    } // while 
    
    return true ;
  } // PureList()

  bool IsDefined( TreePtr head, int & pos ) {
    int j = gUseFunc.size()-1 ;
    for ( ; j >= 0 ; j-- ) {
      if ( IsEqual( gUseFunc.at( j ).defined, head ) ) {
        pos = j ;
        return true ;
      } // if
    } // for
    
    pos = -1 ;
    return false ;
  } // IsDefined()
  
  int FindFuncSet( TreePtr head, int & place ) {
    int j = gUseFunc.size() - 1, last = j ;
    bool global = true ;
    bool inLambda = true ;
    if ( place != -1 ) {
      global = false ;
      j = place-1 ;
      if ( j >= 0 && gUseFunc.at( j ).name == "is defined" )
        inLambda = false ;
    } // if
      
    bool inLet = false ;
    bool find = true ;
    if ( j >= 0 && gUseFunc.at( j ).type == LET )
      inLet = true ;

    for ( ; j >= 0 ; j-- ) {
      if ( inLet || ( j == last && gUseFunc.at( j ).name != "not defined" ) )
        for ( int i = 0 ; i < gUseFunc.at( j ).args.size() ; i++ ) {
          if ( head->token.tokenStr == gUseFunc.at( j ).args.at( i ).name ) {
            place = i ;
            return j ;
          } // if 
        } // for
        
      if ( j-1 >= 0 && gUseFunc.at( j ).type != LET ) 
        inLet = false ;
        
      if ( j == last && gUseFunc.at( j ).name == "not defined" )
        last-- ;
    } // for 

    return -1 ;
  } // FindFuncSet()
  
  TreePtr FindBound( TreePtr head, int & position, bool & isEval ) {
    int j = gUseFunc.size() - 1, last = j ;
    bool global = true ;
    bool inLambda = true ;
    if ( position != -1 ) {
      global = false ;
      j = position-1 ;
      if ( gUseFunc.at( j ).name == "is defined" )
        inLambda = false ;
    } // if
      
    bool inLet = false ;
    bool find = true ;
    if ( j >= 0 && gUseFunc.at( j ).type == LET )
      inLet = true ;
    for ( ; j >= 0 ; j-- ) {
      if ( inLet || ( j == last && gUseFunc.at( j ).name != "not defined" ) )
        for ( int i = 0 ; i < gUseFunc.at( j ).args.size() ; i++ ) {
          if ( head->token.tokenStr == gUseFunc.at( j ).args.at( i ).name ) {
            position = j ;
            isEval = true ;
            return gUseFunc.at( j ).args.at( i ).defined ;
          } // if 
        } // for
        
      if ( j-1 >= 0 && gUseFunc.at( j ).type != LET ) 
        inLet = false ;
        
      if ( j == last && gUseFunc.at( j ).name == "not defined" )
        last-- ;
    } // for 
    
    map<string,Define>::iterator result = gDefineTree.find( head->token.tokenStr ) ;
    if ( result != gDefineTree.end() ) {
      position = -1 ; 
      isEval = result->second.isEval ;
      return result->second.defined ;
    } // if
    
    result = gSystemSymbol.find( head->token.tokenStr ) ;
    if ( result != gSystemSymbol.end() ) {
      position = -1 ;
      isEval = false ;
      return result->second.defined ;
    } // if

    return NULL ;
  } // FindBound()
  
  TreePtr FindDefineBound( TreePtr head ) {  
    map<string,Define>::iterator result = gDefineTree.find( head->token.tokenStr ) ;
    if ( result != gDefineTree.end() )
      return result->second.defined ;   
    return NULL ;
  } // FindDefineBound()
  
  TreePtr FindBound( string token ) {   
    for ( int j = gUseFunc.size()-1 ; j >= 0 ; j-- ) {
      for ( int i = 0 ; i < gUseFunc.at( j ).args.size() ; i++ ) {
        if ( token == gUseFunc.at( j ).args.at( i ).name ) {
          return Copy( gUseFunc.at( j ).args.at( i ).defined, NULL ) ;
        } // if 
      } // for
    } // for
    
    
    map<string,Define>::iterator result = gDefineTree.find( token ) ;
    if ( result != gDefineTree.end() )
      return Copy( result->second.defined, NULL ) ;
    result = gSystemSymbol.find( token ) ;
    if ( result != gSystemSymbol.end() )
      return Copy( result->second.defined, NULL ) ;

    return NULL ;
  } // FindBound()
  
  TreePtr Copy( TreePtr cur, TreePtr parent ) {
    if ( cur == NULL )
      return NULL ;   
    TreePtr copy = new Tree() ;
    copy->exist = cur->exist ;
    copy->token = cur->token ;
    copy->protect = cur->protect ;
    copy->parent = parent ;
    copy->left = Copy( cur->left, cur ) ;
    copy->right = Copy( cur->right, cur ) ;
    return copy ; 
  } // Copy()
  
  string IntNum( int num ) {
    string token = "" ;
    stringstream sstream;
    sstream << num ;
    token = sstream.str() ;
    return token ;
  } // IntNum()
  
  string FloatNum( double temp ) {
    string token = "" ;
    if ( temp != ( int ) temp ) {
      temp = temp * 1000 ;
      temp = round( temp ) ;
      temp = temp / 1000 ;
    } // if
    
    stringstream sstream;
    sstream << temp ;
    token = sstream.str() ;
    bool find = false ;
    string final = "" ;
    int pos = 0 ;
    while ( !find ) {
      if ( pos >= token.length() ) {
        find = true ;
        final = final + "." ;
      } // if
      else {
        final = final + token.at( pos ) ;
        if ( token.at( pos ) == '.' ) {
          find = true ;
          if ( pos == 0 )
            final = "0" + final ;
        } // if
        
        pos++ ;
      } // else
    } // while
    
    int count = 0 ;
    int i = pos ;
    for ( ; i < token.length() && count < 3 ; i++ ) {
      count++ ;
      final = final + token.at( i ) ;
    } // for
    
    int num = token.length() - pos ;
    for ( ; num < 3 ; num++ ) {
      final = final + "0" ;
    } // for
    
    return final ;
  } // FloatNum()
   
  bool IsAtom( TerminalType type ) {
    if ( IsAtomNotSym( type ) )
      return true ;
    else if ( type == SYMBOL )
      return true ;
    else 
      return false ;
  } // IsAtom()
  
  TreePtr Set( string token, TerminalType type ) {
    TreePtr temp = new Tree ;
    temp->exist = true ;
    temp->protect = false ;
    temp->left = NULL ;
    temp->right = NULL ;
    temp->token.tokenStr = token ;
    temp->token.tType = type ;    
    return temp ;
  } // Set()
  
  bool IsNum( TerminalType type ) {
    if ( type == INT )
      return true ;
    else if ( type == FLOAT )
      return true ;
    else
      return false ;
  } // IsNum()
  
  bool IsBool( TerminalType type ) {
    if ( type == T )
      return true ;
    else if ( type == NIL )
      return true ;
    else
      return false ;
  } // IsBool()
  
  bool IsEqual( TreePtr tree1, TreePtr tree2 ) {
    string cmp1 = "" ;
    string cmp2 = "" ;
    Find( tree1, cmp1 ) ;
    Find( tree2, cmp2 ) ;
    if ( cmp1 == cmp2 )
      return true ;
    else
      return false ;
  } // IsEqual()
  
  int IsFunc( TreePtr head, TreePtr cur ) {
    if ( cur->left != NULL ) {
      map<string,Func>::iterator result = gFuncDefine.find( cur->left->token.tokenStr ) ;
      if ( result != gFuncDefine.end() )
        return 1 ;
    } // if
    
    return 0 ;
  } // IsFunc()
  
  void Change( TreePtr & cur, string token ) {
    // change defined symbol to its binding
    if ( cur == NULL ) ;
    else if ( cur->token.tokenStr == token )
      cur = FindBound( token ) ;
    else {
      TreePtr temp ;
      Change( cur->left, token ) ;
      Change( cur->right, token ) ;
    } // else
  } // Change()
  
  TreePtr Cond( TreePtr head, TreePtr cur, bool copy, int & correct ) {
    TreePtr cond = Eval( head, cur->left, copy ) ;
    if ( cond == NULL ) {
      correct = -1 ;
      return NULL ;
    } // if
    else if ( cond->token.tokenStr == "no return value" ) {
      throw MyException( cond->token.tokenStr, gNoReturn, 11 ) ;
    } // else if
    else if ( cond->token.tType == NIL ) {
      correct = 0 ;
      return NULL ;
    } // else if 
    else {
      TreePtr temp = cur->right ;
      TreePtr res ;
      while ( temp != NULL ) {
        if ( temp->token.tokenStr != "nil" ) {
          TreePtr get = Eval( head, temp->left, copy ) ;
          if ( get == NULL ) {
            correct = -1 ;
            return NULL ;
          } // if
          else
            res = get ;
        } // if 
        
        temp = temp->right ;
      } // while
      
      correct = 1 ;
      return res ;
    } // else
  } // Cond()
  
  TreePtr EvalCons( vector <TreePtr> args, bool copy ) {
    TreePtr temp = new Tree ;
    temp->protect = false ;
    temp->token.tType = NOTHING ;
    TreePtr s2 = args.at( 0 ) ;
    temp->left = s2 ;
    TreePtr s3 = args.at( 1 ) ;
    temp->right = s3 ;
    return temp ;
  } // EvalCons()
  
  TreePtr EvalQuote( vector <TreePtr> args, bool copy ) {
    return args.at( 0 ) ;
  } // EvalQuote()
  
  TreePtr EvalList( vector <TreePtr> args, bool copy ) {
    TreePtr res = new Tree ;
    TreePtr walk = res ;
    res->exist = true ;
    res->protect = false ;
    res->token.tokenStr = "(" ;
    res->token.tType = LEFTP ;
    res->left = NULL ;
    res->right = NULL ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      walk->left = args.at( i ) ;
      walk->right = new Tree ;
      walk = walk->right ;
      walk->exist = false ;
      walk->protect = false ;
      walk->token.tType = NOTHING ;
      walk->left = NULL ;
      walk->right = NULL ;
    } // for
      
    walk->token.tokenStr = "nil" ;
    walk->token.tType = NIL ;
      
    return res ;
  } // EvalList()
  
  TreePtr EvalSet( TreePtr head, TreePtr cur, bool copy ) {
    int place = -1 ;
    int pos = FindFuncSet( cur->right->left, place ) ;
    map<string,Define>::iterator define ;
    if ( pos == -1 )
      define = gDefineTree.find( cur->right->left->token.tokenStr ) ;     
    TreePtr res = Eval( head, cur->right->right->left, copy ) ;
    /*
    cout << "----------------debug---------------\n" ;
    cout << cur->right->left->token.tokenStr << "\n" ;
    cout << pos << " " ;
    if ( pos != -1 )
      cout << gUseFunc.at( pos ).name  ;
    cout<< "\n" ;
    Print( res, 0 , false ) ;
    cout << "-----------------finish-------------\n" ;
    */
    if ( pos != -1 ) {
      if ( res != NULL ) {
        if ( res->token.tokenStr == "no return value" ) {
          gNoReturn = Copy( cur->right->right->left, NULL ) ;
          return Set( "no return value", SYMBOL ) ;
        } // if
        
        gUseFunc.at( pos ).args.at( place ).defined = Copy( res, NULL ) ;
        return res ;
      } // if
      else
        return NULL ;
    } // if
    else if ( define != gDefineTree.end() ) {
      if ( res != NULL ) {
        if ( res->token.tokenStr == "no return value" ) {
          gNoReturn = Copy( cur->right->right->left, NULL ) ;
          return Set( "no return value", SYMBOL ) ;
        } // if
        
        map<string,Func>::iterator check = gFuncDefine.find( define->first ) ;
        if ( check != gFuncDefine.end() )
          gFuncDefine.erase( check ) ;      
        Protect( res ) ;
        define->second.defined = res ;
        define->second.isEval = true ;
        return res ;
      } // if
      else
        return NULL ;
    } // if
    else {
      Define temp ;
      temp.name = cur->right->left->token.tokenStr ;
      if ( res != NULL ) {
        if ( res->token.tokenStr == "no return value" ) {
          gNoReturn = Copy( cur->right->right->left, NULL ) ;
          return Set( "no return value", SYMBOL ) ;
        } // if

        Protect( res ) ;
        temp.defined = res ;
        temp.isEval = true ;
        gDefineTree[temp.name] = temp ;
        return res ;
      } // if
      else
        return NULL ;
    } // else
  } // EvalSet()
  
  void Protect( TreePtr & head ) {
    if ( head != NULL ) {
      head->protect = true ;
      Protect( head->left ) ;
      Protect( head->right ) ;
    } // if
  } // Protect()
  
  TreePtr EvalDefine( TreePtr head, TreePtr cur, bool copy ) {
    if ( cur->right->left->token.tType == SYMBOL ) {
      map<string,Define>::iterator pos = gDefineTree.find( cur->right->left->token.tokenStr ) ;
      TreePtr res = Eval( head, cur->right->right->left, copy ) ;
      if ( pos != gDefineTree.end() ) {
        if ( res != NULL ) {
          if ( res->token.tokenStr == "no return value" ) {
            gNoReturn = Copy( cur->right->right->left, NULL ) ;
            return Set( "no return value", SYMBOL ) ;
          } // if
          
          TreePtr defined = cur->right->right->left ;
          
          int find = IsFunc( head, defined ) ;
          if ( find == 1 ) {
            defined = res ;
            pos->second.isEval = true ;
          } // if
          else if ( gHasRead ) {
            defined = res ;
            pos->second.isEval = true ;
          } // else if 
          else {
            Change( defined, cur->right->left->token.tokenStr ) ;
            pos->second.isEval = false ;
          } // else  
          
          map<string,Func>::iterator check = gFuncDefine.find( pos->first ) ;
          if ( check != gFuncDefine.end() )
            gFuncDefine.erase( check ) ;
          Protect( defined ) ;
          pos->second.defined = defined ;
          if ( gVerbose )
            cout << pos->first << " defined\n" ;
            
          return NULL ;
        } // if
        else
          return NULL ;
      } // if
      else {
        Define temp ;
        temp.name = cur->right->left->token.tokenStr ;
        if ( res != NULL ) {
          if ( res->token.tokenStr == "no return value" ) {
            gNoReturn = Copy( cur->right->right->left, NULL ) ;
            return Set( "no return value", SYMBOL ) ;
          } // if
          
          TreePtr defined = cur->right->right->left ;
          int find = IsFunc( head, defined ) ;
          if ( find == 1 ) {
            defined = res ;
            temp.isEval = true ;
          } // if
          else if ( gHasRead ) {
            defined = res ;
            temp.isEval = true ;
          } // else if 
          else {
            Change( defined, cur->right->left->token.tokenStr ) ;
            temp.isEval = false ;
          } // else
          
          Protect( defined ) ;
          temp.defined = defined ;
          gDefineTree[temp.name] = temp ;
          if ( gVerbose )
            cout << temp.name << " defined\n" ; 
            
          return NULL ;
        } // if
        else
          return NULL ; 
      } // else
    } // if
    else {
      TreePtr walk = cur->right->left ;
      Func temp ;
      bool first = true ;
      while ( walk != NULL ) {
        if ( walk->token.tokenStr != "nil" ) {
          if ( walk->left->token.tType != SYMBOL ) {
            throw MyException( "define", head, 6 ) ;
          } // if
          else if ( first ) {
            if ( IsPrimitive( walk->left->token.tokenStr ) ) {
              throw MyException( "define", head, 6 ) ;
            } // if
            else
              temp.name = walk->left->token.tokenStr ;
              
            first = false ;
          } // else if
          else {
            Define tempD ;
            tempD.name = walk->left->token.tokenStr ;
            tempD.defined = NULL ;
            temp.args.push_back( tempD ) ;
          } // else
                
        } // if 
              
        walk = walk->right ;
      } // while
            
      temp.num = temp.args.size() ;
      Protect( cur->right->right ) ;
      temp.defined = cur->right->right ;
      temp.type = FUNC ;
      
      map<string,Define>::iterator pos = gDefineTree.find( temp.name ) ;
      if ( pos != gDefineTree.end() ) {
        TreePtr tempT = new Tree ;
        tempT->exist = true ;
        tempT->protect = false ;
        tempT->left = NULL ;
        tempT->right = NULL ;
        tempT->parent = NULL ;
        tempT->token.tokenStr = "#<procedure " + temp.name + ">" ;
        tempT->token.tType = STRING ;
        pos->second.defined = tempT ;
      } // if 
      else
        MakeFuncDefine( temp.name ) ;
        
      map<string,Func>::iterator func = gFuncDefine.find( temp.name ) ;
      if ( func == gFuncDefine.end() ) {
        if ( gVerbose )
          cout << temp.name << " defined\n" ; 
          
        gFuncDefine[temp.name] = temp ;
      } // if   
      else {
        if ( gVerbose )
          cout << temp.name << " defined\n" ;
          
        func->second = temp ;
      } // else

      return NULL ;
    } // else
  } // EvalDefine()
  
  TreePtr EvalLambda( TreePtr head, TreePtr cur, bool copy ) {
    TreePtr compute = gUseFunc.at( gUseFunc.size()-1 ).defined ;
    // Set Args 
    TreePtr walk = cur->right ;
    int i = 0, last = gUseFunc.size()-1 ;
    while ( walk != NULL ) {
      if ( walk->token.tokenStr != "nil" ) {
        TreePtr res = Eval( head, walk->left, copy ) ;
        if ( res == NULL )
          return NULL ;
        else if ( res->token.tokenStr == "no return value" ) {
          throw MyException( res->token.tokenStr, gNoReturn, 9 ) ;
        } // else if

        gUseFunc.at( gUseFunc.size()-1 ).args.at( i ).defined = res ;
        i++ ;
      } // if  
      
      walk = walk->right ;
    } // while
    // Evaluate

    gUseFunc.at( gUseFunc.size()-1 ).name = "is defined" ;
    TreePtr result = NULL ;
    while ( compute != NULL ) {
      if ( compute->token.tokenStr != "nil" ) {
        TreePtr get = Eval( head, compute->left, copy ) ;
        if ( get == NULL )
          return NULL ;
        else if ( get->token.tokenStr == "no return value" ) {
          if ( compute->right != NULL && compute->right->token.tokenStr == "nil" ) {
            gNoReturn = Copy( cur, NULL ) ;
            gUseFunc.erase( gUseFunc.end() ) ;
            return Set( "no return value", SYMBOL ) ;
          } // if
        } // else if
        else
          result = get ;
      } // if 
      
      compute = compute->right ;
    } // while
    
    /*
    cout << "=======\nresult : " ;
    Print( result, 0 ) ;
    cout << "==================\n" ;
    */
    gUseFunc.erase( gUseFunc.end() ) ;
    return result ;
  } // EvalLambda()
  
  TreePtr EvalCar( vector <TreePtr> args, bool copy ) {
    TreePtr s = args.at( 0 ) ;
    return s->left ;
  } // EvalCar()
  
  TreePtr EvalCdr( vector <TreePtr> args, bool copy ) {
    TreePtr s = args.at( 0 ) ;
    return s->right ;
  } // EvalCdr()
  
  TreePtr EvalIsAtom( vector <TreePtr> args ) {
    TreePtr get = args.at( 0 ) ;
    if ( IsAtom( get->token.tType ) )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsAtom()
  
  TreePtr EvalIsPair( vector <TreePtr> args ) {
    TreePtr get = args.at( 0 ) ;
    if ( !IsAtom( get->token.tType ) )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsPair()
  
  TreePtr EvalIsList( vector <TreePtr> args ) {
    TreePtr get = args.at( 0 ) ;
    if ( get->token.tType == NIL )
      return Set( "#t", T ) ;
    else if ( IsAtom( get->token.tType ) ) ;
    else if ( PureList( get ) )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsList()
  
  TreePtr EvalIsNull( vector <TreePtr> args ) {
    TreePtr get = args.at( 0 ) ;
    if ( get->token.tType == NIL )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsNull()
  
  TreePtr EvalIsInt( vector <TreePtr> args ) {
    bool isInt = false ;
    TreePtr get = args.at( 0 ) ;
    if ( get->token.tType == INT )
      return Set( "#t", T ) ;
    else if ( get->token.tType == FLOAT ) {
      double num = get->token.num ;
      if ( num == ( int ) num )
        return Set( "#t", T ) ;
    } // else if
    
    return Set( "nil", NIL ) ;
  } // EvalIsInt()
  
  TreePtr EvalIsNum( vector <TreePtr> args ) {
    bool isReal = false ;
    TreePtr get = args.at( 0 ) ;
    if ( IsNum( get->token.tType ) )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsNum()
  
  TreePtr EvalIsStr( vector <TreePtr> args ) {
    TreePtr get = args.at( 0 ) ;
    if ( get->token.tType == STRING )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsStr() 
  
  TreePtr EvalIsBool( vector <TreePtr> args ) {
    TreePtr get = args.at( 0 ) ;
    if ( IsBool( get->token.tType ) )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsBool() 
  
  TreePtr EvalIsSymbol( vector <TreePtr> args ) {
    TreePtr get = args.at( 0 ) ;
    if ( get->token.tType == SYMBOL )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsSymbol()
  
  TreePtr EvalPlus( vector <TreePtr> args ) {
    double floatNum = 0.0 ;
    int intNum = 0 ;
    bool isInt = true ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr get = args.at( i ) ;
      if ( isInt && get->token.tType == INT ) {
        int tempNum = atoi( get->token.tokenStr.c_str() ) ;
        intNum += tempNum ; 
      } // if
      else if ( isInt && get->token.tType == FLOAT ) {
        double tempNum = get->token.num ;
        floatNum = intNum ;
        floatNum = floatNum + tempNum ;
        isInt = false ;
      } // else if
      else {
        double tempNum = get->token.num ;
        if ( get->token.tType != FLOAT )
          tempNum = atoi( get->token.tokenStr.c_str() ) ;
        floatNum = floatNum + tempNum ;
      } // else
    } // for
    
    TreePtr res = new Tree ; 
    res->protect = false ;
    res->exist = true ;
    res->left = NULL ;
    res->right = NULL ;
    if ( !isInt ) {
      res->token.tokenStr = FloatNum( floatNum ) ;
      res->token.tType = FLOAT ;
      res->token.num = floatNum ;
    } // if
    else {
      res->token.tokenStr = IntNum( intNum ) ;
      res->token.tType = INT ;
    } // else
    
    return res ;
  } // EvalPlus()
  
  TreePtr EvalMinus( vector <TreePtr> args ) {
    double floatNum = 0.0 ;
    int intNum = 0 ;
    bool isInt = true, first = true ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr get = args.at( i ) ;
      if ( isInt && get->token.tType == INT ) {
        int tempNum = atoi( get->token.tokenStr.c_str() ) ;
        if ( first )
          intNum = tempNum ;
        else
          intNum = intNum - tempNum ; 
      } // if
      else if ( isInt && get->token.tType == FLOAT ) {
        double tempNum = get->token.num ;
        floatNum = intNum ;
        if ( first )
          floatNum = tempNum ;
        else
          floatNum = floatNum - tempNum ;
              
        isInt = false ;
      } // else if
      else {
        double tempNum = get->token.num ;
        if ( get->token.tType != FLOAT )
          tempNum = atoi( get->token.tokenStr.c_str() ) ;
        if ( first )
          floatNum = tempNum ;
        else
          floatNum = floatNum - tempNum ;
      } // else
          
      first = false ;
    } // for
    
    TreePtr res = new Tree ; 
    res->protect = false ;
    res->exist = true ;
    res->left = NULL ;
    res->right = NULL ;
    if ( !isInt ) {
      res->token.tokenStr = FloatNum( floatNum ) ;
      res->token.tType = FLOAT ;
      res->token.num = floatNum ;
    } // if
    else {
      res->token.tokenStr = IntNum( intNum ) ;
      res->token.tType = INT ;
    } // else
    
    return res ;
  } // EvalMinus()
  
  TreePtr EvalMul( vector <TreePtr> args ) {
    double floatNum = 1.0 ;
    int intNum = 1 ;
    bool isInt = true ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr get = args.at( i ) ;
      if ( isInt && get->token.tType == INT ) {
        int tempNum = atoi( get->token.tokenStr.c_str() ) ;
        intNum = intNum * tempNum ; 
      } // if
      else if ( isInt && get->token.tType == FLOAT ) {
        double tempNum = get->token.num ;
        floatNum = intNum ;
        floatNum = floatNum * tempNum ;
        isInt = false ;
      } // else if
      else {
        double tempNum = get->token.num ;
        if ( get->token.tType != FLOAT )
          tempNum = atoi( get->token.tokenStr.c_str() ) ;
        floatNum = floatNum * tempNum ;
      } // else
    } // for
    
    TreePtr res = new Tree ; 
    res->protect = false ;
    res->exist = true ;
    res->left = NULL ;
    res->right = NULL ;
    if ( !isInt ) {
      res->token.tokenStr = FloatNum( floatNum ) ;
      res->token.tType = FLOAT ;
      res->token.num = floatNum ;
    } // if
    else {
      res->token.tokenStr = IntNum( intNum ) ;
      res->token.tType = INT ;
    } // else
    
    return res ;
  } // EvalMul()
  
  TreePtr EvalDiv( TreePtr cur, vector <TreePtr> args ) {
    double floatNum = 1.0 ;
    int intNum = 1 ;
    bool isInt = true, first = true ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr get = args.at( i ) ;
      if ( isInt && get->token.tType == INT ) {
        int tempNum = atoi( get->token.tokenStr.c_str() ) ;
        if ( first )
          intNum = tempNum ;
        else {
          if ( tempNum == 0 ) {
            throw MyException( get->token.tokenStr, cur, 13 ) ;
          } // if
              
          intNum = intNum / tempNum ; 
        } // else
      } // if
      else if ( isInt && get->token.tType == FLOAT ) {
        double tempNum = get->token.num ;
        floatNum = intNum ;
        isInt = false ;
        if ( first )
          floatNum = tempNum ;
        else {
          if ( tempNum == 0 ) {
            throw MyException( get->token.tokenStr, cur, 13 ) ;
          } // if
              
          floatNum = floatNum / tempNum ;
        } // else  
      } // else if
      else {
        double tempNum = get->token.num ;
        if ( get->token.tType != FLOAT )
          tempNum = atoi( get->token.tokenStr.c_str() ) ;
        if ( first )
          floatNum = tempNum ;
        else {
          if ( tempNum == 0 ) {
            throw MyException( get->token.tokenStr, cur, 13 ) ;
          } // if
              
          floatNum = floatNum / tempNum ;
        } // else    
      } // else
          
      first = false ;
    } // for
    

    TreePtr res = new Tree ; 
    res->protect = false ;
    res->exist = true ;
    res->left = NULL ;
    res->right = NULL ;
    if ( !isInt ) {
      res->token.tokenStr = FloatNum( floatNum ) ;
      res->token.tType = FLOAT ;
      res->token.num = floatNum ;
    } // if
    else {
      res->token.tokenStr = IntNum( intNum ) ;
      res->token.tType = INT ;
    } // else
    
    return res ;
  } // EvalDiv()
  
  bool CompareNum( double num1, double num2, string type ) {
    if ( type == ">" )
      if ( num1 > num2 )
        return true ;
      else
        return false ;
    else if ( type == ">=" )
      if ( num1 >= num2 )
        return true ;
      else
        return false ;
    else if ( type == "<" )
      if ( num1 < num2 )
        return true ;
      else
        return false ;
    else if ( type == "<=" )
      if ( num1 <= num2 )
        return true ;
      else
        return false ;
    else if ( type == "=" )
      if ( num1 == num2 )
        return true ;
      else
        return false ;
    else    
      return false ;
  } // CompareNum()
  
  TreePtr EvalCmpNum( vector <TreePtr> args, string func ) {
    double floatNum = 0.0 ;
    bool compare = false, is = true ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr get = args.at( i ) ;
      if ( !compare ) {
        floatNum = atof( get->token.tokenStr.c_str() ) ;
        compare = true ;
      } // if
      else {
        double cmp = atof( get->token.tokenStr.c_str() ) ;
        if ( !CompareNum( floatNum, cmp, func ) )
          is = false ;
        else
          floatNum = cmp ;
      } // else
    } // for
    
    if ( is )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalCmpNum()
  
  TreePtr EvalStrAppend( vector <TreePtr> args ) {
    string str = "" ;
    bool first = true ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr get = args.at( i ) ;
      if ( first ) {
        str = get->token.tokenStr ;
        first = false ;
      } // if
      else {
        str.erase( str.end()-1 ) ;
        string copy = get->token.tokenStr ;
        copy.erase( copy.begin() ) ;
        str = str + copy ;
      } // else
    } // for
    
    TreePtr res = new Tree ; 
    res->protect = false ;
    res->exist = true ;
    res->left = NULL ;
    res->right = NULL ;
    res->token.tokenStr = str ;
    res->token.tType = STRING ;     
    return res ;
  } // EvalStrAppend()
  
  bool CompareStr( string str1, string str2, string type ) {
    if ( type == "string>?" )
      if ( str1 > str2 )
        return true ;
      else
        return false ;
    else if ( type == "string<?" )
      if ( str1 < str2 )
        return true ;
      else
        return false ;
    else if ( type == "string=?" )
      if ( str1 == str2 )
        return true ;
      else
        return false ;
    else
      return false ;
  } // CompareStr()
  
  TreePtr EvalCmpStr( vector <TreePtr> args, string func ) {
    string cmp = "" ;
    bool first = true ;
    bool is = true ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr get = args.at( i ) ;
      if ( first ) {
        cmp = get->token.tokenStr ;
        cmp.erase( cmp.begin() ) ;
        cmp.erase( cmp.begin()+cmp.length()-1 ) ;
        first = false ;
      } // if 
      else {
        string copy = get->token.tokenStr ;
        copy.erase( copy.begin() ) ;
        copy.erase( copy.begin()+copy.length()-1 ) ;
        if ( !CompareStr( cmp, copy, func ) )
          is = false ;
        else
          cmp = copy ;
      } // else 
    } // for
    
    if ( is )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalCmpStr()
  
  TreePtr EvalIsEqv( TreePtr head, TreePtr cur, bool copy ) {
    if ( IsAtomNotSym( cur->right->left->token.tType ) 
         && IsAtomNotSym( cur->right->right->left->token.tType ) ) {
      if ( cur->right->left->token.tokenStr == cur->right->right->left->token.tokenStr
           && cur->right->left->token.tType != STRING )
        return Set( "#t", T ) ; 
      else
        return Set( "nil", NIL ) ;
    } // if
    
    TreePtr cmp1 = Eval( head, cur->right->left, false ) ;
    if ( cmp1 != NULL ) {
      if ( cmp1->token.tokenStr == "no return value" ) {
        throw MyException( cmp1->token.tokenStr, gNoReturn, 9 ) ;
      } // if
      
      TreePtr cmp2 = Eval( head, cur->right->right->left, false ) ;
      if ( cmp2 != NULL ) { 
        if ( cmp2->token.tokenStr == "no return value" ) {
          throw MyException( cmp2->token.tokenStr, gNoReturn, 9 ) ;
        } // if
        
        if ( cmp1 == cmp2 )
          return Set( "#t", T ) ;
        else
          return Set( "nil", NIL ) ;
      } // if 
      else
        return NULL ;
    } // if
    
    else
      return NULL ;
  } // EvalIsEqv()
  
  TreePtr EvalIsEqual( vector <TreePtr> args ) {
    TreePtr cmp1 = args.at( 0 ) ;
    TreePtr cmp2 = args.at( 1 ) ;
    if ( IsEqual( cmp1, cmp2 ) )
      return Set( "#t", T ) ;
    return Set( "nil", NIL ) ;
  } // EvalIsEqual() 
  
  TreePtr EvalAnd( TreePtr head, TreePtr cur, bool copy ) {
    TreePtr temp = cur->right ;
    TreePtr res ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) {
        TreePtr get = Eval( head, temp->left, copy ) ;
        if ( get == NULL )
          return NULL ;
        else if ( get->token.tokenStr == "no return value" ) {
          throw MyException( get->token.tokenStr, temp->left, 12 ) ;
        } // else if
        else if ( get->token.tokenStr == "nil" )
          return get ;
        else
          res = get ;
      } // if 
      
      temp = temp->right ;
    } // while
    
    return res ;
  } // EvalAnd()
  
  TreePtr EvalOr( TreePtr head, TreePtr cur, bool copy ) {
    TreePtr temp = cur->right ;
    TreePtr res ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) {
        TreePtr get = Eval( head, temp->left, copy ) ;
        if ( get == NULL )
          return NULL ;
        else if ( get->token.tokenStr == "no return value" ) {
          throw MyException( get->token.tokenStr, temp->left, 12 ) ;
        } // else if
        else if ( get->token.tokenStr != "nil" )
          return get ;
        else
          res = get ;
      } // if 
      
      temp = temp->right ;
    } // while
    
    return res ;
  } // EvalOr()
  
  TreePtr EvalIf( TreePtr head, TreePtr cur, bool copy ) {
    TreePtr cond = Eval( head, cur->right->left, copy ) ;
    if ( cond == NULL )
      return NULL ;
    else if ( cond->token.tokenStr == "no return value" ) {
      throw MyException( cond->token.tokenStr, gNoReturn, 11 ) ;
    } // else if
    else {
      int num = GetArgsNum( cur->right ) ;  
      if ( cond->token.tType != NIL ) {
        TreePtr isTrue = Eval( head, cur->right->right->left, copy ) ;
        if ( isTrue == NULL )
          return NULL ;
        else if ( isTrue->token.tokenStr == "no return value" ) {
          gNoReturn = Copy( cur, NULL ) ;
          return Set( "no return value", SYMBOL ) ;
        } // if
        
        return isTrue ;
      } // if     
      else {
        TreePtr isFalse = NULL ;
        if ( num == 2 ) {
          gNoReturn = Copy( cur, NULL ) ;
          return Set( "no return value", SYMBOL ) ;
        } // if
        
        isFalse = Eval( head, cur->right->right->right->left, copy ) ;
        if ( isFalse == NULL )
          return NULL ;
        else if ( isFalse->token.tokenStr == "no return value" ) {
          gNoReturn = Copy( cur, NULL ) ;
          return Set( "no return value", SYMBOL ) ;
        } // if
        
        return isFalse ;
      } // else
    } // else if
  } // EvalIf()
  
  TreePtr EvalCond( TreePtr head, TreePtr cur, bool copy ) {
    TreePtr walk = cur->right ;
    for ( ; walk != NULL ; walk = walk->right ) {
      if ( walk->token.tokenStr == "nil" ) ;
      else if ( walk->right->token.tokenStr != "nil" ) {
        int correct = -1 ;        // -1:error, 0:not correct, 1:correct
        TreePtr get = Cond( head, walk->left, copy, correct ) ;
        if ( correct == -1 )
          return NULL ;
        else if ( correct == 1 )
          return get ;
      } // if
      else {      // ordinary
        TreePtr ordinary = walk->left ;
        if ( ordinary->left->token.tokenStr == "else" 
             || ordinary->left->token.tType == T ) {
          TreePtr temp = walk->left->right ;
          TreePtr res ;
          while ( temp != NULL ) {
            if ( temp->token.tokenStr != "nil" ) {
              TreePtr get = Eval( head, temp->left, copy ) ;
              if ( get == NULL )
                return NULL ;
              else
                res = get ;
            } // if 
      
            temp = temp->right ;
          } // while
    
          return res ;
        } // if
        else {
          TreePtr check = Eval( head, ordinary->left, copy ) ;
          if ( check == NULL ) 
            return NULL ;
          else if ( check->token.tokenStr == "no return value" )
            throw MyException( check->token.tokenStr, gNoReturn, 11 ) ;
          else if ( check->token.tType == NIL ) {
            gNoReturn = Copy( cur, NULL ) ;
            return Set( "no return value", SYMBOL ) ;
          } // else if
          else {
            TreePtr temp = walk->left->right ;
            TreePtr res ;
            while ( temp != NULL ) {
              if ( temp->token.tokenStr != "nil" ) {
                TreePtr get = Eval( head, temp->left, copy ) ;
                if ( get == NULL )
                  return NULL ;
                else
                  res = get ;
              } // if 
      
              temp = temp->right ;
            } // while
          
            return res ;
          } // else
        } // else
      } // else
    } // for
    
    return NULL ;
  } // EvalCond()
  
  TreePtr EvalLet( TreePtr head, TreePtr cur, bool copy ) {
    TreePtr walk = cur->right->left ;
    Func temp ;
    temp.name = "let" ;
    temp.type = LET ;
    temp.num = 0 ;
    while ( walk != NULL ) {
      if ( walk->token.tokenStr != "nil" ) {
        Define store ;          
        store.name = walk->left->left->token.tokenStr ;
        TreePtr res = Eval( head, walk->left->right->left, copy ) ;
        if ( res->token.tokenStr == "no return value" ) {
          throw MyException( res->token.tokenStr, gNoReturn, 8 ) ;
        } // if
        
        store.defined = res ;
        temp.args.push_back( store ) ;
        temp.num++ ;
      } // if
      
      walk = walk->right ;
    } // while

    gUseFunc.push_back( temp ) ;
    int position = gUseFunc.size()-1 ;
    walk = cur->right->right ;
    TreePtr res ;
    while ( walk != NULL ) {
      if ( walk->token.tokenStr != "nil" ) {
        TreePtr get = Eval( walk->left, walk->left, copy ) ;
        if ( get != NULL && get->token.tokenStr == "no return value" ) {
          if ( walk->right->token.tokenStr == "nil" ) {
            gNoReturn  = Copy( cur, NULL ) ;
            gUseFunc.erase( gUseFunc.end() ) ;
            return Set( "no return value", SYMBOL ) ;
          } // if
        } // if
        else
          res = get ;
      } // if 
      
      walk = walk->right ;
    } // while
    
    gUseFunc.erase( gUseFunc.begin()+position ) ;
    return res ;
  } // EvalLet()
  
  TreePtr EvalBegin( TreePtr head, TreePtr cur, bool copy ) {
    TreePtr temp = cur->right ;
    TreePtr res ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) {
        TreePtr get = Eval( head, temp->left, copy ) ;
        if ( get == NULL ) 
          res = get ;
        else if ( get->token.tokenStr == "no return value" ) {
          if ( temp->right != NULL && temp->right->token.tokenStr == "nil" ) {
            gNoReturn = Copy( cur, NULL ) ;
            return Set( "no return value", SYMBOL ) ;
          } // if
        } // else if
        else
          res = get ;
      } // if 
      
      temp = temp->right ;
    } // while

    return res ;
  } // EvalBegin() 
  
  TreePtr EvalFunc( TreePtr head, TreePtr cur, string func, 
                    vector <TreePtr> args, bool copy ) {   
    vector <TreePtr> store ;
    for ( int i = 0 ; i < args.size() ; i++ ) {
      TreePtr res = Eval( head, args.at( i ), copy ) ;
      if ( res == NULL )
        return NULL ;
      else if ( res->token.tokenStr == "no return value" ) {
        throw MyException( res->token.tokenStr, gNoReturn, 9 ) ;
      } // else if
      
      store.push_back( res ) ;
    } // for
    
    map<string,Func>::iterator pos = gFuncDefine.find( func ) ;
    gUseFunc.push_back( pos->second ) ;
    for ( int i = 0 ; i < store.size() ; i++ ) {
      gUseFunc.at( gUseFunc.size()-1 ).args.at( i ).defined = store.at( i ) ;
    } // for
    
    
    int position = gUseFunc.size()-1 ;
    TreePtr res ;
    TreePtr isBound = pos->second.defined ;
    while ( isBound != NULL ) {
      if ( isBound->token.tokenStr != "nil" ) {
        res = Eval( isBound->left, isBound->left, copy ) ;
        if ( res != NULL && res->token.tokenStr == "no return value" ) {
          if ( isBound->right->token.tokenStr == "nil" )
            gNoReturn = cur ;  
        } // if
      } // if 
       
      isBound = isBound->right ;
    } // while
    
    gUseFunc.erase( gUseFunc.begin()+position ) ;
    return res ;
  } // EvalFunc()
  
  TreePtr Read() {
    MakeTree tree ; 
    vector <TokenS> expr ;
    bool end = false, res ;
    string message = "" ;
    while ( expr.size() == 0 && !end ) {
      res = ReadSExp( expr, end, tree, true, message ) ;  
    } // while
    
    if ( expr.size() == 0 && end && message == "" && res )
      message = "ERROR : END-OF-FILE encountered when there should be more input" ;

    if ( message != "" )
      return Set( message, ERROR ) ;
    else
      return tree.GetTree() ;
  } // Read()
  
  TreePtr Evaluate( TreePtr head, TreePtr cur, string func, bool copy, 
                    vector <TreePtr> args ) {
    if ( func == "cons" ) 
      return EvalCons( args, copy ) ;
    else if ( func == "quote" )
      return EvalQuote( args, copy ) ;
    else if ( func == "list" )
      return EvalList( args, copy ) ;
    else if ( func == "define" )
      return EvalDefine( head, cur, copy ) ;
    else if ( func == "lambda" )
      return EvalLambda( head, cur, copy ) ;
    else if ( func == "car" ) 
      return EvalCar( args, copy ) ;
    else if ( func == "cdr" )
      return EvalCdr( args, copy ) ;
    else if ( func == "atom?" )
      return EvalIsAtom( args ) ;
    else if ( func == "pair?" )
      return EvalIsPair( args ) ;
    else if ( func == "list?" )
      return EvalIsList( args ) ;
    else if ( func == "null?" )
      return EvalIsNull( args ) ;
    else if ( func == "integer?" )
      return EvalIsInt( args ) ;
    else if ( func == "real?" || func == "number?" )
      return EvalIsNum( args ) ;
    else if ( func == "string?" )
      return EvalIsStr( args ) ;
    else if ( func == "boolean?" )
      return EvalIsBool( args ) ; 
    else if ( func == "symbol?" )
      return EvalIsSymbol( args ) ; 
    else if ( func == "/" )
      return EvalDiv( cur, args ) ;
    else if ( func == "not" ) {
      bool isNot = false ;
      TreePtr get = args.at( 0 ) ;
      if ( get == NULL )
        return NULL ;
      if (  get->token.tType == NIL )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == ">" || func == ">=" || func == "<" 
              || func == "<=" || func == "=" )
      return EvalCmpNum( args, func ) ;
    else if ( func == "string-append" )
      return EvalStrAppend( args ) ;
    else if ( func == "string>?" || func == "string<?" || func == "string=?" )
      return EvalCmpStr( args, func ) ;
    else if ( func == "eqv?" )
      return EvalIsEqv( head, cur, copy ) ;
    else if ( func == "equal?" )
      return EvalIsEqual( args ) ;
    else if ( func == "cond" )
      return EvalCond( head, cur, copy ) ;
    else if ( func == "let" )
      return EvalLet( head, cur, copy ) ;
    else if ( func == "clean-environment" ) {
      gDefineTree.clear() ;
      gFuncDefine.clear() ;
      gUseFunc.clear() ;
      if ( gVerbose )
        cout << "environment cleaned\n" ;
      return NULL ;
    } // else if
    else if ( func == "verbose?" ) {
      if ( gVerbose )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "verbose" ) {
      if ( args.at( 0 )->token.tType == NIL ) {
        gVerbose = false ;
        return Set( "nil", NIL ) ;
      } // if 
      else {
        gVerbose = true ;
        return Set( "#t", T ) ;
      } // else
    } // else if
    else if ( func == "create-error-object" ) {
      return Set( args.at( 0 )->token.tokenStr, ERROR ) ;
    } // else if
    else if ( func == "error-object?" ) {
      if ( args.at( 0 )->token.tType == ERROR )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else
      return EvalFunc( head, cur, func, args, copy ) ;
    
    return NULL ;
  } // Evaluate()
  
  TreePtr EvaluateOperate( TreePtr head, string func, vector <TreePtr> args ) {
    if ( func == "+" )
      return EvalPlus( args ) ; 
    else if ( func == "-" )
      return EvalMinus( args ) ;
    else if ( func == "*" )
      return EvalMul( args ) ;
    else
      return NULL ;
  } // EvaluateOperate()
  
  TreePtr EvaluateCondition( TreePtr head, TreePtr cur, string func, bool copy ) {
    if ( func == "and" )
      return EvalAnd( head, cur, copy ) ;
    else if ( func == "or" )
      return EvalOr( head, cur, copy ) ;
    else if ( func == "if" )
      return EvalIf( head, cur, copy ) ;
    else if ( func == "begin" )
      return EvalBegin( head, cur, copy ) ;
    else
      return NULL ;
  } // EvaluateCondition()
  
  TreePtr EvaluateWrite( TreePtr head, string func, vector <TreePtr> args ) {
    if ( func == "write" ) {
      Print( args.at( 0 ), 0, true ) ;
      return args.at( 0 ) ;
    } // if
    else if ( func == "display-string" ) {
      string res = args.at( 0 )->token.tokenStr ;
      res.erase( res.begin() ) ;
      res.erase( res.end()-1 ) ;
      cout << res ;
      return args.at( 0 ) ;
    } // else if
    else if ( func == "newline" ) {
      cout << "\n" ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "symbol->string" ) {
      string quote = "\"" ;
      string res = args.at( 0 )->token.tokenStr ;
      res = quote + res + quote ;
      return Set( res, STRING ) ;
    } // else if
    else if ( func == "number->string" ) {
      string quote = "\"" ;
      string res = args.at( 0 )->token.tokenStr ;
      res = quote + res + quote ;
      return Set( res, STRING ) ;
    } // else if
    else
      return NULL ;
  } // EvaluateWrite()
  
  TreePtr Eval( TreePtr head, TreePtr cur, bool copy ) {
    /*
    if ( gCount == 343 ) {
      cout << "\n-------------------Eval--------------------\n" ;
    cout << gUseFunc.size() << "\n";
    Print( cur, 0, false ) ; 
    for ( int i = 0 ; i < gUseFunc.size() ; i++ ) {
      cout << gUseFunc.at( i ).name << "\n" ;
    } // for
    cout << "\n" ;   
    }
           
    */
    string token ;
    vector <TreePtr> args ;
    bool isArgs = false ;
    if ( IsAtomNotSym( cur->token.tType ) ) {
      return cur ;
    } // if
    else if ( cur->token.tType == SYMBOL ) {
      int place = -1 ;
      bool eval = false ;
      TreePtr isBound = FindBound( cur, place, eval ) ;
      if ( isBound == NULL ) {
        throw MyException( cur->token.tokenStr, cur, 1 ) ;
      } // if
      else {
        if ( eval )  
          return isBound ;
        else 
          return Eval( isBound, isBound, false ) ;
      } // else  
    } // else if 
    else {
      if ( !PureList( cur ) ) {
        throw MyException( cur->token.tokenStr, cur, 2 ) ;
        return NULL ;
      } // if
      else if ( IsAtomNotSym( cur->left->token.tType ) ) {
        throw MyException( cur->left->token.tokenStr, cur, 3 ) ;
      } // if
      else if ( cur->left->token.tType == SYMBOL || cur->left->token.tokenStr == "quote" ) {
        int type = 0 ;
        token = cur->left->token.tokenStr ;
        if ( IsKnown( token ) ) {
          if ( head != cur && ( token == "clean-environment"
                                || token == "define" 
                                || token == "exit" ) ) {
            throw MyException( token, cur, 4 ) ;
          } // if
          else if ( token == "exit" ) {
            if ( !CheckArgsEqual( cur->right, 0 ) )
              throw MyException( token, cur, 5 ) ;
            
            gExit = true ;
            return NULL ;
          } // else if 
          else if ( token == "define" ) {
            int num = GetArgsNum( cur->right ) ;
            if ( num < 2 ) {
              throw MyException( token, cur, 6 ) ;
            } // if
            else if ( IsAtomNotSym( cur->right->left->token.tType ) ) {
              throw MyException( token, cur, 6 ) ;
            } // if
            else if ( cur->right->left->token.tType == SYMBOL ) {
              if ( num != 2 ) {
                throw MyException( token, cur, 6 ) ;
              } // if
              else if ( IsPrimitive( cur->right->left->token.tokenStr ) ) {
                throw MyException( token, cur, 6 ) ;
              } // if
            } // if
                                              
            TreePtr result = EvalDefine( head, cur, copy ) ;
            return result ;
          } // else if
          else if ( token == "set!" ) {
            if ( GetArgsNum( cur->right ) != 2 )
              throw MyException( token, cur, 6 ) ;
            if ( cur->right->left->token.tType != SYMBOL )
              throw MyException( token, cur, 6 ) ;
            if ( IsPrimitive( cur->right->left->token.tokenStr ) )
              throw MyException( token, cur, 6 ) ;
            
            TreePtr result = EvalSet( head, cur, copy ) ;
            return result ;
          } // else if 
          else if ( token == "cond" ) {
            if ( !CheckArgsNum( cur, "cond" ) ) {
              throw MyException( token, cur, 6 ) ;
            } // if

            TreePtr temp = cur->right ;
            int num = GetArgsNum( cur->right ) ;
            for ( ; temp != NULL ; temp = temp->right ) {  
              if ( temp->token.tokenStr == "nil" ) ;             
              else if ( !CheckArgsBigger( temp->left, 2 ) || !PureList( temp->left ) ) {
                throw MyException( token, cur, 6 ) ;
              } // if
            } // for
            
            TreePtr result = EvalCond( head, cur, copy ) ;
            return result ;
          } // else if
          else if ( token == "lambda" ) {
            if ( !CheckArgsBigger( cur->right, 2 ) ) {
              throw MyException( token, cur, 6 ) ;
            } // if
            else if ( cur->right->left->token.tokenStr != "(" 
                      && cur->right->left->token.tokenStr != "nil" ) {
              throw MyException( token, cur, 6 ) ;
            } // else if
            else if ( !CheckSymbol( cur->right->left ) ) {
              throw MyException( token, cur, 6 ) ;
            } // else if
            
            
            int pos = -1, num = 0 ;

            TreePtr walk = cur->right->left ;
            Func res ;  
            while ( walk != NULL ) {
              if ( walk->token.tokenStr != "nil" ) {
                Define store ;
                store.name = walk->left->token.tokenStr ;
                store.defined = NULL ;         
                res.args.push_back( store ) ;
                num++ ;
              } // if
        
              walk = walk->right ;
            } // while
      
            res.name = "not defined" ;
            res.type = LAMBDA ;
            res.defined = Copy( cur->right->right, NULL ) ;
            res.num = res.args.size() ;
            gUseFunc.push_back( res ) ;
            return Set( "#<procedure lambda>", SYMBOL ) ;
          } // else if
          else if ( token == "let" ) {
            if ( !CheckArgsBigger( cur->right, 2 ) ) {
              throw MyException( token, cur, 6 ) ;
            } // if

            TreePtr temp = cur->right->left ;
            while ( temp != NULL ) {   
              if ( temp->token.tokenStr != "nil" ) {
                if ( GetArgsNum( temp->left ) != 2 || !PureList( temp->left ) ) {
                  throw MyException( token, cur, 6 ) ;
                } // if
                else if ( temp->left->left->token.tType != SYMBOL ) {
                  throw MyException( token, cur, 6 ) ;
                } // else if
                else if ( IsPrimitive( temp->left->left->token.tokenStr ) )
                  throw MyException( token, cur, 6 ) ;            
              } // if  
              
              temp = temp->right ;
            } // while
            
            TreePtr result = EvalLet( head, cur, copy ) ;
            return result ;
          } // else if
          else if ( token == "if" || token == "and" || token == "or" || token == "begin" ) {
            if ( !CheckArgsNum( cur, token ) ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            TreePtr result = EvaluateCondition( head, cur, token, copy ) ;
            return result ;
          } // else if
          else if ( token == "read" ) {
            if ( GetArgsNum( cur->right ) > 0 ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            gHasRead = true ;
            return Read() ;
          } // else if
          else if ( token == "eval" ) {
            if ( GetArgsNum( cur->right ) != 1 ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            TreePtr res = Eval( head, cur->right->left, copy ) ;
            return Eval( res, res, copy ) ;
          } // else if
          else {      // not define, cond
            if ( !CheckArgsNum( cur, token ) ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            if ( token == "eqv?" )
              copy = false ;
            
            map<string,Func>::iterator find = gFuncDefine.find( token ) ;
            if ( find != gFuncDefine.end() )
              isArgs = true ;
          } // else
        } // if
        else {
          int find = -1 ;
          bool eval ;
          TreePtr isBound = FindBound( cur->left, find, eval ) ;
          if ( isBound == NULL ) {
            throw MyException( cur->left->token.tokenStr, cur, 1 ) ;
          } // if
          
          TreePtr get = Eval( head, isBound, copy ) ; 
          if ( get == NULL )
            return NULL ;
          token = ChangeType( get->token.tokenStr ) ;
          if ( IsKnown( token ) && !IsPrimitive( get->token.tokenStr ) ) {
            if ( token == "lambda" ) {
              int num = GetArgsNum( cur->right ) ;
              int numLam = gUseFunc.at( gUseFunc.size()-1 ).num ;
              if ( num != numLam ) {
                throw MyException( token, cur, 5 ) ;
              } // if
              
              isArgs = true ;
            } // if
            else if ( !CheckArgsNum( cur, token ) ) {
              throw MyException( token, cur, 5 ) ;
            } // else if
            
            if ( token == "eqv?" )
              copy = false ;
              
            map<string,Func>::iterator find = gFuncDefine.find( token ) ;
            if ( find != gFuncDefine.end() )
              isArgs = true ;
          } // if
          else if ( token == "no return value" ) {
            throw MyException( token, gNoReturn, 8 ) ;
          } // if
          else {
            TreePtr res = get ; 
            throw MyException( token, res, 7 ) ;
          } // else 
        } // else
      } // else if 
      else {
        int size = gUseFunc.size() ;
        TreePtr noError = Eval( head, cur->left, copy ) ;
        if ( noError != NULL ) {
          token = ChangeType( noError->token.tokenStr ) ;
          if ( token == "lambda" ) {
            int num = GetArgsNum( cur->right ) ;
            int numLam = gUseFunc.at( gUseFunc.size()-1 ).num ;
            if ( num != numLam ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            isArgs = true ;
          } // if
          else if ( IsKnown( token ) ) {
            if ( !CheckArgsNum( cur, token ) ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            if ( token == "eqv?" )
              copy = false ;
          } // if
          else {
            if ( token == "no return value" ) {
              throw MyException( token, gNoReturn, 8 ) ;
            } // if
            else {
              TreePtr res = noError ;
              throw MyException( cur->token.tokenStr, res, 7 ) ;
            } // else
            
            return NULL ;
          } // else
        } // if
        else
          return NULL ;
      } // else
    } // else
    
    TreePtr temp = cur->right ;
    while ( temp != NULL ) {
      if ( temp->token.tokenStr != "nil" ) {
        bool type = true ;
        TreePtr arg = NULL ;
        if ( token == "quote" ) 
          arg = temp->left ;
        else if ( isArgs )
          arg = temp->left ;
        else 
          arg = Eval( head, temp->left, copy ) ;  
                
        if ( arg == NULL )
          return NULL ;
        else if ( arg->token.tokenStr == "no return value" )
          throw MyException( token, gNoReturn, 9 ) ;
        else if ( token == "car" || token == "cdr" ) {
          if ( IsAtom( arg->token.tType ) || arg->token.tType == ERROR )
            throw MyException( token, arg, 10 ) ;
        } // else if
        else if ( token == "+" || token == "-" || token == "*" || token == "/" || token == "=" 
                  || token == ">" || token == ">=" || token == "<" || token == "<=" ) {
          if ( !IsNum( arg->token.tType ) )
            throw MyException( token, arg, 10 ) ;
        } // else if
        else if ( token == "string-append" || token == "string>?" || token == "string<?"
                  || token == "string=?" ) {
          if ( arg->token.tType != STRING && arg->token.tType != ERROR )
            throw MyException( token, arg, 10 ) ;
        } // else if
        else if ( token == "display-string" || token == "create-error-object" ) {
          if ( arg->token.tType != STRING && arg->token.tType != ERROR )
            throw MyException( token, arg, 10 ) ;
        } // else if
        else if ( token == "symbol->string" ) {
          if ( arg->token.tType != SYMBOL )
            throw MyException( token, arg, 10 ) ;
        } // else if
        else if ( token == "number->string" ) {
          if ( !IsNum( arg->token.tType ) )
            throw MyException( token, arg, 10 ) ;
        } // else if
        
        args.push_back( arg ) ;
      } // if  
      
      temp = temp->right ;
    } // while
    
    
    
    TreePtr result ;
    if ( token == "write" || token == "display-string" || token == "newline"
         || token == "symbol->string" || token == "number->string" )
      result = EvaluateWrite( head, token, args ) ;
    else if ( token == "+" || token == "-" || token == "*" )
      result = EvaluateOperate( head, token, args ) ;
    else
      result = Evaluate( head, cur, token, copy, args ) ;
      
    if ( result != NULL )
      if ( result->token.tokenStr == "(" 
           && result->left == NULL && result->right == NULL ) {
        result->token.tokenStr = "nil" ;
        result->token.tType = NIL ;
      } // if
            
    return result ;
  } // Eval()
};

bool ReadSExp( vector <TokenS> & expr, bool & end,
               MakeTree & makeTree, bool read, string & message ) {
  Scanner scan ;  
  TokenS token ;
  
  try {
    if ( scan.SExp( expr, true, token ) ) {
      scan.Paren( expr ) ; 
      makeTree.Create( expr ) ;  
    } // if
    
    if ( expr.size() > 0 ) {
      gLastLine = gAbsLine ;
      gNextLine = 1 ;
      if ( gChar != ' ' && gChar != '\t' )
        gNextColumn = 1 ; 
      else
        gNextColumn = 2 ;
      gLine = gNextColumn ;
      gColumn = gNextColumn ;
    } // if
    else {
      if ( gLastLine == gAbsLine ) {
        gNextLine = 1 ;
        gNextColumn = 1 ; 
      } // if 
    } // else
  } // try
  catch ( MyExceptionInput & e ) {
    gSecond = false ; 
    expr.clear() ;
    if ( !read )
      e.Print() ;
    else
      message = e.Set() ;
    end = false ;
    if ( e.IsEnd() )
      end = true ;
      
    scan.Skip() ;
    if ( gLastLine == gAbsLine ) {
      gNextLine = 1 ;
      gNextColumn = 1 ; 
    } // if
    
    return true ;
  } // catch()

  return false ;
} // ReadSExp()

void PrintSExp( vector <TokenS> expr, TreePtr res ) {  
  MakeTree tree ;
  MakeIns ins ;
  if ( res != NULL && res->token.tokenStr == "no return value" ) {
    cout << "ERROR (no return value) : " ;
    ins.Print( gNoReturn, 0, false ) ;
    cout << "\n" ;
  } // if
  else {
    tree.Print( res, 0 ) ;
    if ( expr.size() > 0 && !gExit )
      cout << "\n" ;
  } // else
} // PrintSExp() 

int main() {
  MakeTree tree ;
  MakeIns ins ; 
  int uTestNum = 0 ;
  char ch = '\0' ;
  scanf( "%d%c", &uTestNum, &ch ) ;
  cout << "Welcome to OurScheme!\n\n" ;
  vector <TokenS> expr ;
  bool end = false ;
  string message = "" ;
  ins.Set() ;
  int count = 0 ;
  while ( !gExit && !end ) {
    cout << "> " ;
    bool error = ReadSExp( expr, end, tree, false, message ) ;
    bool clear = true ;
    if ( expr.size() > 0 ) {
      gHasRead = false ;
      TreePtr temp = tree.GetTree() ;
      if ( !error ) {
        try {
          TreePtr result = ins.Copy( ins.Eval( temp, temp, true ), NULL ) ;
          PrintSExp( expr, result ) ;
          tree.Delete( result ) ; 
        } // try
        catch( MyException & e ) {
          e.Error() ;
        } // catch
        
        gUseFunc.clear() ;
      } // if

      tree.Delete( temp ) ;
    } // if

    expr.clear() ;
    tree.Build() ;
  } // while()
  
  cout << "\nThanks for using OurScheme!" ; 
} // main() 
