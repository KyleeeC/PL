# include <cstdio>
# include <cstdlib>
# include <cmath>
# include <iostream>
# include <string>
# include <vector>
# include <sstream>
# include <ctype.h>

using namespace std ; 

enum TerminalType { SYMBOL = 1000, INT = 500, FLOAT = 400, STRING = 300, NIL = 200, T = 100, 
                    LEFTP = 50, RIGHTP = 40, QUOTE = 20, DOT = 10
};

enum FunctionType { FUNC = 3000, LAMBDA = 2000, LET = 1000 } ; 


char gChar = '\0', gLastChar = '\0' ;

int gLine = 1, gColumn = 1 ;
int gNextLine = 1, gNextColumn = 1 ;
int gAbsNextLine = 1, gAbsLine = 1 ;
bool gParen = false ;

struct TokenS {
  int line ;
  int column ;
  string tokenStr ;     // StyleCheckType string
  double num ;
  TerminalType tType ;
};

struct Tree {
  bool exist ;
  bool isDot ;
  TokenS token ;
  Tree * left ;
  Tree * right ;
  Tree * parent ;
};

typedef Tree * TreePtr ;

struct Define {
  string name ;
  TreePtr defined ;
};

struct Func {
  string name ;
  int num ;
  FunctionType type ;
  vector <Define> args ;
  TreePtr defined ;
  bool lam ;
};

vector <Define> gDefineTree ;
vector <Define> gSystemSymbol ;
vector <Func> gFuncDefine ;
vector <Func> gUseFunc ;
vector <Func> gLambda ;
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



class Scanner {
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
  
  int SExp( vector <TokenS> tknS, int & pos, vector <TokenS> & expr, int start, int & type, 
            TokenS & err, bool final, bool & backToTop ) {
    // error type1 && type3 : expected '('
    // error type2 : expected ')' 
    int test = Atom( tknS, pos, expr, final ) ;
    if ( test == 1 )
      return 1 ;
    else if ( pos >= tknS.size() && !final )
      return 2 ;
    else if ( pos >= tknS.size() && final )
      return 0 ;
    else if ( tknS.at( pos ).tokenStr == "(" ) {
      expr.push_back( tknS.at( pos ) ) ;
      start = pos+1 ;
      pos++ ;
      if ( pos >= tknS.size() && !final )
        return 2 ;
      else if ( pos >= tknS.size() && final )
        return 0 ;
      else {
        int make1 = SExp( tknS, pos, expr, start, type, err, final, backToTop ) ;
        if ( make1 == 2 )
          return 2 ;
        else if ( make1 == 0 ) {
          backToTop = true ;
          return 0 ;
        } // else if
        else {
          start = pos ;
          if ( pos >= tknS.size() && !final )
            return 2 ;
          else if ( pos >= tknS.size() && final )
            return 0 ;
            
          int make2 = SExp( tknS, pos, expr, start, type, err, final, backToTop ) ;
          while ( make2 == 1 ) {
            start = pos ;
            if ( pos >= tknS.size() )
              return 2 ; 
            
            make2 = SExp( tknS, pos, expr, start, type, err, final, backToTop ) ;
          } // while 
          
          if ( make2 == 2 )
            return 2 ;
          else if ( backToTop )
            return 0 ;
          
          pos = start ;         
          if ( pos >= tknS.size() )
            return 2 ;  
          else if ( tknS.at( pos ).tType == DOT ) {
            expr.push_back( tknS.at( pos ) ) ;
            pos++ ;
            start = pos+1 ;
            if ( pos >= tknS.size() && !final )
              return 2 ;
            else if ( pos >= tknS.size() && final )
              return 0 ;
              
            int make3 = SExp( tknS, pos, expr, start, type, err, final, backToTop ) ;
            if ( make3 == 0 ) {
              backToTop = true ;
              return 0 ;
            } // if
            else if ( make3 == 2 )
              return 2 ;  
          } // if

          if ( pos >= tknS.size() && !final )
            return 2 ;
          else if ( pos >= tknS.size() && final )
            return 0 ;
          else if ( tknS.at( pos ).tokenStr == ")" ) {
            expr.push_back( tknS.at( pos ) ) ;
            pos++ ;
            return 1 ;
          } // if
          else {
            type = 2 ;
            backToTop = true ;
            err = tknS.at( pos ) ;
            return 0 ;  
          } // else
        } // else
      } // else  
    } // else if
    else if ( tknS.at( pos ).tokenStr == "\'" ) {
      expr.push_back( tknS.at( pos ) ) ;
      start = pos+1 ;
      pos++ ;
      if ( pos >= tknS.size() && !final )
        return 2 ;
      else if ( pos >= tknS.size() && final )
        return 0 ;
        
      int make4 = SExp( tknS, pos, expr, start, type, err, final, backToTop ) ;
      if ( make4 == 0 )
        backToTop = true ;
      return make4 ; 
    } // else if
    else {
      type = 1 ;
      err = tknS.at( pos ) ;
      return 0 ;
    } // else
      
  } // SExp()
  
  int Atom( vector <TokenS> tknS, int & pos, vector <TokenS> & expr, bool final ) {
    if ( pos >= tknS.size() && !final )
      return 2 ;
    else if ( pos >= tknS.size() && final )
      return 0 ;
    else if ( tknS.at( pos ).tType == SYMBOL ) {
      expr.push_back( tknS.at( pos ) ) ;
      pos++ ;
      return 1 ;
    } // if
    else if ( tknS.at( pos ).tType == INT ) {
      expr.push_back( tknS.at( pos ) ) ;
      pos++ ;
      return 1 ;
    } // if
    else if ( tknS.at( pos ).tType == FLOAT ) {
      expr.push_back( tknS.at( pos ) ) ;
      pos++ ;
      return 1 ;
    } // if
    else if ( tknS.at( pos ).tType == STRING ) {
      expr.push_back( tknS.at( pos ) ) ;
      pos++ ;
      return 1 ;
    } // if
    else if ( tknS.at( pos ).tType == NIL ) {
      expr.push_back( tknS.at( pos ) ) ;
      pos++ ;
      return 1 ;
    } // if
    else if ( tknS.at( pos ).tType == T ) {
      expr.push_back( tknS.at( pos ) ) ;
      pos++ ;
      return 1 ;
    } // if
    else if ( tknS.at( pos ).tokenStr == "(" ) {
      if ( pos+1 >= tknS.size() )
        return 0 ;
      else if ( tknS.at( pos+1 ).tokenStr == ")" ) {
        expr.push_back( tknS.at( pos ) ) ;
        expr.push_back( tknS.at( pos+1 ) ) ;
        pos = pos + 2 ;
        return 1 ;
      } // if
      else
        return 0 ;
    } // if
    else
      return 0 ;               
  } // Atom()
 
public: 
  bool Skip() {
    bool change = false ;
    int line, column ;
    if ( gChar == '\n' )
      change = true ; 
      
    while ( !change ) {
      if ( !GetChar( line, column ) )
        return false ;
            
      if ( gChar == '\n' )
        change = true ; 
    } // while
    
    gNextLine = 1 ;
    gNextColumn = 1 ;
    return true ;
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
        gAbsNextLine++ ;
        gNextLine++ ;
        gNextColumn = 1 ;  
      } // else
  
      return true ;
    } // if
    else if ( gLastChar != '\n' && !gParen ) {
      gChar = '\n' ;
      line = gNextLine ;
      column = gNextColumn ;
      gLine = gNextLine ;
      gColumn = gNextColumn ;
      gAbsLine = gAbsNextLine ;
      gAbsNextLine++ ;
      gNextLine++ ;
      gNextColumn = 1 ;
      return false ;
    }  // else if
    else
      return false ;  
      
  } // GetChar()

  bool Grammer( vector <TokenS> & tknS, vector <TokenS> & expr, int & type, TokenS & err, bool final ) {
    // success : 0->error, 1->success, 2->not finish getting token
    int pos = 0 ;
    bool back = false ;
    int success = SExp( tknS, pos, expr, 0, type, err, final, back ) ; 
    if ( success == 0 )
      return false ;
    else
      return true ;  
  } // Grammer()
   
  bool MakeToken( vector <TokenS> & tknS, int & paren, bool & quoteE, bool & com, 
                  int & line, int & column, bool & second, bool & quote, bool & dQuote ) {
    quoteE = false, dQuote = false ;
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
          dQuote = true ;   // jump and dont't read
          tempS = backup ;
          jump = true ;
        } // else
      } // if
      else if ( gChar == '\"' && special ) {
        special = false ;
        jump = true ;
        dQuote = false ;
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
        quoteE = true ; 
      } // if
       
    }  // while
    
    // -----------------------------------finish get token--------------------
    
    if ( !notEOF && paren > 0 ) {
      return false ;
    } // if
    
    if ( gChar == ';' ) {
      do {
        if ( !GetChar( line, column ) )
          notEOF = false ;
            
        if ( gChar == '\n' )
          change = true ; 
      } while ( !change && notEOF ) ;
    } // if
    
    temp.tokenStr = tempS ;
    if ( gChar == ';' && tempS == "" )
      com = true ;
    else
      com = false ;  
      
    if ( quoteE )
      temp.tokenStr = "" ;
    else if ( temp.tokenStr == "" ) ;  
    else if ( IsInt( tempS ) ) {
      int tempI = atoi( temp.tokenStr.c_str() ) ;
      stringstream sstream;
      sstream << tempI ;
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = sstream.str() ;
      temp.tType = INT ;
      quote = false ;
    } // if
    else if ( IsString( tempS ) ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = STRING ;
      quote = false ;
    } // else if
    else if ( tempS == "." ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = DOT ;
      quote = false ;
    } // else if
    else if ( IsFloat( tempS ) ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = FloatNum( tempS ) ; 
      temp.tType = FLOAT ;
      temp.num = My_atof( tempS ) ;
      quote = false ;
    } // else if
    else if ( tempS == "nil" || tempS == "#f" ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = "nil" ;
      temp.tType = NIL ;
      quote = false ;
    } // else if
    else if ( tempS == "t" || tempS == "#t" ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tokenStr = "#t" ;
      temp.tType = T ;
      quote = false ;
    } // else if

    else {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = SYMBOL ;
      quote = false ;
    } // else

    second = false ;
    if ( temp.tokenStr != "" )
      tknS.push_back( temp ) ;
   
    if ( quoteE ) ;
    else if ( gChar == '(' ) {
      quote = false ;
      if ( !second && temp.tokenStr != "" && paren == 0 ) {
        second = true ;
      } // if
      else {
        paren++ ;
        temp.line = gLine ;
        temp.column = gColumn ;
        temp.tokenStr = "(" ;
        temp.tType = LEFTP ;
        tknS.push_back( temp ) ;
      } // else
    }  // if
    else if ( gChar == ')' ) {
      if ( !second && temp.tokenStr != "" && paren == 0 ) {
        second = true ;
      } // if
      else {
        paren-- ;
        temp.line = gLine ;
        temp.column = gColumn ;
        temp.tokenStr = ")" ;
        temp.tType = RIGHTP ;
        tknS.push_back( temp ) ;
      } // else
    } // else if
    else if ( gChar == '\'' ) {
      if ( !second && temp.tokenStr != "" && paren == 0 ) {
        second = true ;
      } // if
      else {
        quote = true ;
        temp.line = gLine ;
        temp.column = gColumn ;
        temp.tokenStr = "\'" ;
        temp.tType = QUOTE ;
        tknS.push_back( temp ) ;
      } // else if
    } // else if

    return notEOF ;
    
  } // MakeToken()
  
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
  void Find( TreePtr tree, string & str ) {
    if ( tree == NULL ) ;
    else if ( tree->exist ) {
      if ( tree->isDot && tree->token.tType == LEFTP ) {
        str = str + "(" ;
        Find( tree->left, str ) ;
        str = str + "." ;
        Find( tree-> right, str ) ;    
        str = str + ")" ;
      } // if
      else if ( tree->isDot && tree->token.tType == DOT ) {
        Find( tree->left, str ) ;
        str = str + "." ;
        Find( tree-> right, str ) ; 
      } // if    
      else if ( tree->token.tType == LEFTP ) {
        str = str + "(" ;
        Find( tree->left, str ) ;
        Find( tree-> right, str ) ; 
        str = str + ")" ;
      } // if
      else {
        str = str + tree->token.tokenStr ;
      } // else
    } // else if
    else {
      Find( tree->left, str ) ;
      Find( tree->right, str ) ; 
    } // else
  } // Find()
  
public:
  MakeTree() {
    mhead = new Tree ;
    mhead->exist = false ;
    mhead->isDot = false ;
    mhead->left = NULL ;
    mhead->right = NULL ;
    mhead->parent = NULL ;
    mlevel = 0 ;
    mspace = false ;
  } // MakeTree()
  
  void Delete( TreePtr & tree ) {
    tree->parent = NULL ;
    if ( tree != NULL ) {
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
  
  void Print( TreePtr tree, int spaceNum ) {
    if ( tree == NULL ) ;
    else if ( tree->exist ) {
      if ( tree->isDot && tree->token.tType == LEFTP ) {
        if ( !mspace ) {
          for ( int i = 0 ; i < spaceNum ; i++ ) {
            cout << "  " ;
          } // for
        } // if
        
        cout << "( " ;
        mspace = true ;
        Print( tree->left, spaceNum+1 ) ;
        for ( int i = 0 ; i < spaceNum+1 ; i++ )
          cout << "  " ;
          
        cout << "." << endl ;
        Print( tree-> right, spaceNum+1 ) ;
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
        
        cout << ")" << endl ;
        mspace = false ; 
      } // if
      else if ( tree->isDot && tree->token.tType == DOT ) {
        Print( tree->left, spaceNum ) ; 
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
          
        cout << "." << endl ;
        Print( tree->right, spaceNum ) ; 
      } // if    
      else if ( tree->token.tType == LEFTP ) {
        if ( !mspace ) {
          for ( int i = 0 ; i < spaceNum ; i++ ) {
            cout << "  " ;
          } // for
        } // if
        
        cout << "( " ;
        mspace = true ;
        Print( tree->left, spaceNum+1 ) ;
        Print( tree-> right, spaceNum+1 ) ;
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
        
        cout << ")" << endl ;
        mspace = false ;
      } // if
      else {
        if ( mspace ) {
          mspace = false ;
          if ( tree->token.tType != FLOAT )
            cout << tree->token.tokenStr << endl ;
          else
            printf( "%.3f\n", tree->token.num ) ;
        } // if
        else {
          for ( int i = 0 ; i < spaceNum ; i++ )
            cout << "  " ;
      
          if ( tree->token.tType != FLOAT )
            cout << tree->token.tokenStr << endl ;
          else
            printf( "%.3f\n", tree->token.num ) ;
        } // else
      } // else
    } // else if
    else {
      Print( tree->left, spaceNum ) ;
      if ( tree->isDot ) {
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
          
        cout << "." << endl ;
      } // if
      
      Print( tree->right, spaceNum ) ;
    } // else
  } // Print()
  
  void Create( vector <TokenS> expr ) {
    TreePtr parent = mhead ;
    TreePtr temp = mhead ;
    for ( int i = 0 ; i < expr.size() ; i++ ) {
      /*
      Print( mhead, 0 ) ;
      cout << "Current : " << expr.at( i ).tokenStr << endl ;
      for ( int j = 0 ; j < expr.size() ; j++ ) {
        cout << expr.at( j ).tokenStr << " " ;
      }
      cout << endl ;
      cout << "Walk : " << endl;
      for ( TreePtr walk = temp ; walk != NULL ; walk = walk->parent ) {
        cout << "[" << walk->token.tokenStr << "]" << endl ;
      }
      cout << endl << "============" << endl ;
      */
      
      if ( expr.at( i ).tType == LEFTP ) {
        if ( parent->right != NULL && temp == parent->right ) {
          temp->exist = false ;
          temp->isDot = false ;
          temp->left = new Tree ;
          temp->right = NULL ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
        } // if
        
        mlevel++ ;
        temp->exist = true ;
        temp->token = expr.at( i ) ;
        temp->isDot = false ;
        temp->left = new Tree ;
        temp->right = NULL ;
        parent = temp ;
        temp->left->parent = parent ;
        temp = temp->left ;
        temp->isDot = false ;
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
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else if
        else if ( i < expr.size()-1 && expr.at( i+1 ).tType == RIGHTP ) {
          temp->right = new Tree ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->isDot = false ;
          temp->token.tType = NIL ;
          temp->token.tokenStr = "nil" ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else if
        else {    // ATOM
          temp->right = new Tree ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->isDot = false ;
          temp->left = new Tree ;
          temp->right = NULL ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
          temp->exist = false ;
          temp->isDot = false ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else
        
      } // else if
      else if ( expr.at( i ).tType == QUOTE ) {
        if ( parent->right != NULL && temp == parent->right ) {
          temp->exist = false ;
          temp->isDot = false ;
          temp->left = new Tree ;
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
        temp->isDot = false ;
        temp->left = new Tree ;
        temp->left->parent = temp ;
        temp->left->exist = true ;
        temp->left->isDot = false ;
        temp->left->token.tokenStr = "quote" ;
        temp->left->token.tType = SYMBOL ;
        temp->left->left = NULL ;
        temp->left->right = NULL ;
        temp->right = new Tree ;
        parent = temp ;
        temp->right->parent = parent ;
        temp = temp->right ;
        temp->exist = false ;
        temp->isDot = false ;
        temp->left = NULL ;
        temp->right = NULL ;
        TokenS tempS ;
        tempS.tokenStr = ")" ;
        tempS.tType = RIGHTP ;
        if ( i < expr.size()-1 && ( expr.at( i+1 ).tType == LEFTP || expr.at( i+1 ).tType == QUOTE ) ) {  
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
          expr.insert( expr.begin()+i+2, tempS ) ;
          temp->right = new Tree ;
          temp->right->exist = false ;
          temp->right->isDot = false ;
          temp->right->parent = temp ;
          temp->right->token.tokenStr = "nil" ;
          temp->right->token.tType = NIL ;
          temp->right->left = NULL ;
          temp->right->right = NULL ;
          temp->left = new Tree ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
          temp->isDot = false ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else
      } // else if
      else if ( expr.at( i ).tType == DOT ) {
        if ( expr.at( i+1 ).tType == LEFTP ) {
          temp->isDot = false ;
          temp->right = new Tree ;         
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->isDot = false ;
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
            parent = temp ;
            temp->left->parent = parent ;
            temp = temp->left ;
            temp->isDot = false ;
            temp->left = NULL ;
            temp->right = NULL ;
          }  // if
        } // if
        else if ( expr.at( i+1 ).tType == NIL ) {
          temp->isDot = false ;
          temp->right = new Tree ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->isDot = false ;
          temp->token = expr.at( i+1 ) ;
          temp->left = NULL ;
          temp->right = NULL ;
          
          expr.erase( expr.begin() + i ) ;   // erase DOT
          expr.erase( expr.begin() + i ) ;   // erase NIL
          i = i - 1 ;
        } // else if
        else {    // Is ATOM
          temp->exist = true ;
          temp->isDot = true ;
          if ( temp->token.tokenStr != "(" )
            temp->token = expr.at( i ) ;
            
          temp->right = new Tree ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else
      } // if
      else { // ATOM
        temp->exist = true ;
        temp->isDot = false ;
        temp->token = expr.at( i ) ;
        if ( temp->parent != NULL ) {
          temp = temp->parent ;
          parent = parent->parent ;
        } // if 

        if ( i < expr.size()-1 && expr.at( i+1 ).tType == RIGHTP ) {
          if ( temp->right == NULL ) {
            temp->right = new Tree ;
            parent = temp ;
            temp->right->parent = parent ;
            temp = temp->right ;
            temp->exist = false ;
            temp->token.tType = NIL ;
            temp->token.tokenStr = "nil" ;
            temp->isDot = false ;
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
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->isDot = false ;
          temp->left = NULL ;
          temp->right = NULL ; 
        } // else if
        else if ( i < expr.size()-1 && expr.at( i+1 ).tType == DOT ) ;
        else if ( i < expr.size()-1 ) {
          temp->right = new Tree ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->isDot = false ;
          temp->right = NULL ; 
          temp->left = new Tree ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
          temp->isDot = false ;
          temp->left = NULL ;
          temp->right = NULL ;
        } // else if
      } // else
    } // for
    
  } // Create()
  
  bool IsExit() {
    TreePtr temp = mhead ;
    string str = "" ; 
    Find( temp, str ) ;
    if ( str.compare( "(exit)" ) == 0 )
      return true ;
    else  
      return false ;
  } // IsExit()
  
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
    mhead->exist = false ;
    mhead->isDot = false ;
    mhead->left = NULL ;
    mhead->right = NULL ;
    mhead->parent = NULL ;
    mlevel = 0 ;
  } // Build()
  
  TreePtr GetTree() {
    return mhead ;
  } // GetTree()
}; // MakeTree

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
      cout << endl ;
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
      cout << endl ;
    } // else if
    else if ( mtype == 7 ) {
      cout << "ERROR (attempt to apply non-function) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << "\n" ;
    } // else if
    else if ( mtype == 8 ) {
      cout << "ERROR (no return value) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << endl ;
    } // else if
    else if ( mtype == 9 ) {
      cout << "ERROR (unbound parameter) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << endl ;
    } // else if
    else if ( mtype == 10 ) {
      cout << "ERROR (" << mtoken << " with incorrect argument type) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << endl ;
    } // else if
    else if ( mtype == 11 ) {
      cout << "ERROR (unbound test-condition) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << endl ;
    } // else if
    else if ( mtype == 12 ) {
      cout << "ERROR (unbound condition) : " ;
      mtree.Print( mhead, 0 ) ;
      cout << endl ;
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
    if ( token == "#<procedure cons>" )
      return "cons" ;
    else if ( token == "#<procedure list>" )
      return "list" ;
    else if ( token == "#<procedure quote>" )
      return "quote" ;
    else if ( token == "#<procedure define>" )
      return "define" ; 
    else if ( token == "#<procedure lambda>" )
      return "lambda" ;
    else if ( token == "#<procedure car>" )
      return "car" ;
    else if ( token == "#<procedure cdr>" )
      return "cdr" ;  
    else if ( token == "#<procedure atom?>" )
      return "atom?" ;  
    else if ( token == "#<procedure pair?>" )
      return "pair?" ;  
    else if ( token == "#<procedure list?>" )
      return "list?" ;  
    else if ( token == "#<procedure null?>" )
      return "null?" ;  
    else if ( token == "#<procedure integer?>" )
      return "integer?" ;  
    else if ( token == "#<procedure real?>" )
      return "real?" ; 
    else if ( token == "#<procedure number?>" )
      return "number?" ; 
    else if ( token == "#<procedure string?>" )
      return "string?" ; 
    else if ( token == "#<procedure boolean?>" )
      return "boolean?" ; 
    else if ( token == "#<procedure symbol?>" )
      return "symbol?" ; 
    else if ( token == "#<procedure +>" )
      return "+" ; 
    else if ( token == "#<procedure ->" )
      return "-" ; 
    else if ( token == "#<procedure *>" )
      return "*" ; 
    else if ( token == "#<procedure />" )
      return "/" ; 
    else if ( token == "#<procedure not>" )
      return "not" ; 
    else if ( token == "#<procedure and>" )
      return "and" ; 
    else if ( token == "#<procedure or>" )
      return "or" ; 
    else if ( token == "#<procedure >>" )
      return ">" ; 
    else if ( token == "#<procedure >=>" )
      return ">=" ; 
    else if ( token == "#<procedure <>" )
      return "<" ; 
    else if ( token == "#<procedure <=>" )
      return "<=" ; 
    else if ( token == "#<procedure =>" )
      return "=" ; 
    else if ( token == "#<procedure string-append>" )
      return "string-append" ; 
    else if ( token == "#<procedure string>?>" )
      return "string>?" ; 
    else if ( token == "#<procedure string<?>" )
      return "string<?" ; 
    else if ( token == "#<procedure string=?>" )
      return "string=?" ;  
    else if ( token == "#<procedure eqv?>" )
      return "eqv?" ;  
    else if ( token == "#<procedure equal?>" )
      return "equal?" ; 
    else if ( token == "#<procedure begin>" )
      return "begin" ; 
    else if ( token == "#<procedure if>" )
      return "if" ; 
    else if ( token == "#<procedure cond>" )
      return "cond" ; 
    else if ( token == "#<procedure clean-environment>" )
      return "clean-environment" ; 
    else if ( token == "#<procedure let>" )
      return "let" ; 
    else if ( token == "#<procedure exit>" )
      return "exit" ;
    else {
      for ( int i = 0 ; i < gFuncDefine.size() ; i++ ) {
        string temp = "#<procedure " + gFuncDefine.at( i ).name + ">" ;
        if ( temp == token )
          return gFuncDefine.at( i ).name ;
      } // for
      
      return token ; 
    } // else  
  } // ChangeType()
  
  void MakeSystemDefine( string token ) {
    Define temp ;
    temp.name = token ;
    temp.defined = new Tree ;
    temp.defined->exist = true ;
    temp.defined->isDot = false ;
    temp.defined->left = NULL ;
    temp.defined->right = NULL ;
    temp.defined->parent = NULL ;
    temp.defined->token.tokenStr = "#<procedure " + token + ">" ;
    temp.defined->token.tType = STRING ;
    gSystemSymbol.push_back( temp ) ;
  } // MakeSystemDefine()
  
  void MakeFuncDefine( string token ) {
    Define temp ;
    temp.name = token ;
    temp.defined = new Tree ;
    temp.defined->exist = true ;
    temp.defined->isDot = false ;
    temp.defined->left = NULL ;
    temp.defined->right = NULL ;
    temp.defined->parent = NULL ;
    temp.defined->token.tokenStr = "#<procedure " + token + ">" ;
    temp.defined->token.tType = STRING ;
    gDefineTree.push_back( temp ) ;
  } // MakeFuncDefine()
  
  void Set() {
    MakeSystemDefine( "cons" ) ;
    MakeSystemDefine( "list" ) ;
    MakeSystemDefine( "quote" ) ;
    MakeSystemDefine( "define" ) ;
    MakeSystemDefine( "car" ) ;
    MakeSystemDefine( "cdr" ) ;
    MakeSystemDefine( "atom?" ) ;
    MakeSystemDefine( "pair?" ) ;
    MakeSystemDefine( "list?" ) ;
    MakeSystemDefine( "null?" ) ;
    MakeSystemDefine( "integer?" ) ;
    MakeSystemDefine( "real?" ) ;
    MakeSystemDefine( "number?" ) ;
    MakeSystemDefine( "string?" ) ;
    MakeSystemDefine( "boolean?" ) ;
    MakeSystemDefine( "symbol?" ) ;
    MakeSystemDefine( "+" ) ;
    MakeSystemDefine( "-" ) ;
    MakeSystemDefine( "*" ) ;
    MakeSystemDefine( "/" ) ;
    MakeSystemDefine( "not" ) ;
    MakeSystemDefine( "and" ) ;
    MakeSystemDefine( "or" ) ;
    MakeSystemDefine( ">" ) ;
    MakeSystemDefine( ">=" ) ;
    MakeSystemDefine( "<" ) ;
    MakeSystemDefine( "<=" ) ;
    MakeSystemDefine( "=" ) ;
    MakeSystemDefine( "string-append" ) ;
    MakeSystemDefine( "string>?" ) ;
    MakeSystemDefine( "string<?" ) ;
    MakeSystemDefine( "string=?" ) ;
    MakeSystemDefine( "eqv?" ) ;
    MakeSystemDefine( "equal?" ) ;
    MakeSystemDefine( "begin" ) ;
    MakeSystemDefine( "if" ) ;
    MakeSystemDefine( "cond" ) ;
    MakeSystemDefine( "clean-environment" ) ;
    MakeSystemDefine( "exit" ) ;
    MakeSystemDefine( "lambda" ) ;
    MakeSystemDefine( "let" ) ;
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
  
  bool CheckArgsNum( TreePtr cur, string primitive ) {
    int num = GetArgsNum( cur->right ) ;   
    // check args is correct
    if ( primitive == "cons" && num == 2 )
      return true ;
    else if ( primitive == "list" && num >= 0 )
      return true ;
    else if ( primitive == "quote" && num == 1 )
      return true ;
    else if ( primitive == "define" && num == 2 )
      return true ;
    else if ( primitive == "car" && num == 1 )
      return true ;
    else if ( primitive == "cdr" && num == 1 )
      return true ;
    else if ( primitive == "atom?" && num == 1 )
      return true ;
    else if ( primitive == "pair?" && num == 1 )
      return true ;
    else if ( primitive == "list?" && num == 1 )
      return true ;        
    else if ( primitive == "null?" && num == 1 )
      return true ;
    else if ( primitive == "integer?" && num == 1 )
      return true ;
    else if ( primitive == "real?" && num == 1 )
      return true ;
    else if ( primitive == "number?" && num == 1 )
      return true ;
    else if ( primitive == "string?" && num == 1 )
      return true ;
    else if ( primitive == "boolean?" && num == 1 )
      return true ;
    else if ( primitive == "symbol?" && num == 1 )
      return true ;
    else if ( primitive == "+" && num >= 2 )
      return true ;
    else if ( primitive == "-" && num >= 2 )
      return true ;
    else if ( primitive == "*" && num >= 2 )
      return true ;
    else if ( primitive == "/" && num >= 2 )
      return true ;
    else if ( primitive == "not" && num == 1 )
      return true ;
    else if ( primitive == "and" && num >= 2 )
      return true ;
    else if ( primitive == "or" && num >= 2 )
      return true ;
    else if ( primitive == ">" && num >= 2 )
      return true ;
    else if ( primitive == ">=" && num >= 2 )
      return true ;
    else if ( primitive == "<" && num >= 2 )
      return true ;
    else if ( primitive == "<=" && num >= 2 )
      return true ;
    else if ( primitive == "=" && num >= 2 )
      return true ;
    else if ( primitive == "string-append" && num >= 2 )
      return true ;
    else if ( primitive == "string>?" && num >= 2 )
      return true ;
    else if ( primitive == "string<?" && num >= 2 )
      return true ;
    else if ( primitive == "string=?" && num >= 2 )
      return true ;
    else if ( primitive == "eqv?" && num == 2 )
      return true ;
    else if ( primitive == "equal?" && num == 2 )
      return true ;
    else if ( primitive == "begin" && num >= 1 )
      return true ;
    else if ( primitive == "if" && ( num == 2 || num == 3 ) )
      return true ;
    else if ( primitive == "cond" && num >= 1 ) 
      return true ;
    else if ( primitive == "clean-environment" && num == 0 )
      return true ;
    else if ( primitive == "exit" && num == 0 )
      return true ;
    else {
      for ( int i = 0 ; i < gFuncDefine.size() ; i++ ) {
        if ( primitive == gFuncDefine.at( i ).name ) {
          if ( num == gFuncDefine.at( i ).num )
            return true ;
          else
            return false ;
        } // if 
      } // for
      
      return false ; 
    } // else
      
  } // CheckArgsNum()
  
  void Print( TreePtr tree, int spaceNum ) {
    if ( tree == NULL ) ;
    else if ( tree->exist ) {
      if ( tree->isDot && tree->token.tType == LEFTP ) {
        if ( !mspace ) {
          for ( int i = 0 ; i < spaceNum ; i++ ) {
            cout << "  " ;
          } // for
        } // if
        
        cout << "( " ;
        mspace = true ;
        Print( tree->left, spaceNum+1 ) ;
        for ( int i = 0 ; i < spaceNum+1 ; i++ )
          cout << "  " ;
          
        cout << "." << endl ;
        Print( tree-> right, spaceNum+1 ) ;
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
        
        cout << ")" << endl ;
        mspace = false ; 
      } // if
      else if ( tree->isDot && tree->token.tType == DOT ) {
        Print( tree->left, spaceNum ) ; 
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
          
        cout << "." << endl ;
        Print( tree->right, spaceNum ) ; 
      } // if    
      else if ( tree->token.tType == LEFTP ) {
        if ( !mspace ) {
          for ( int i = 0 ; i < spaceNum ; i++ ) {
            cout << "  " ;
          } // for
        } // if
        
        cout << "( " ;
        mspace = true ;
        Print( tree->left, spaceNum+1 ) ;
        Print( tree-> right, spaceNum+1 ) ;
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
        
        cout << ")" << endl ;
        mspace = false ;
      } // if
      else {
        if ( mspace ) {
          mspace = false ;
          if ( tree->token.tType != FLOAT )
            cout << tree->token.tokenStr << endl ;
          else
            printf( "%.3f\n", tree->token.num ) ;
        } // if
        else {
          for ( int i = 0 ; i < spaceNum ; i++ )
            cout << "  " ;
          
          if ( tree->token.tType != FLOAT )
            cout << tree->token.tokenStr << endl ;
          else
            printf( "%.3f\n", tree->token.num ) ;
        } // else
      } // else
    } // else if
    else {
      Print( tree->left, spaceNum ) ;
      if ( tree->isDot ) {
        for ( int i = 0 ; i < spaceNum ; i++ )
          cout << "  " ;
          
        cout << "." << endl ;
      } // if
      
      Print( tree->right, spaceNum ) ;
    } // else
  } // Print()
  
  void Find( TreePtr tree, string & result ) {
    if ( tree == NULL ) ;
    else if ( tree->exist ) {
      if ( tree->isDot && tree->token.tType == LEFTP ) {
        result = result + "(" ;
        Find( tree->left, result ) ;
        result = result + "." ;
        Find( tree-> right, result ) ;
        result = result + ")" ;
      } // if
      else if ( tree->isDot && tree->token.tType == DOT ) {
        Find( tree->left, result ) ; 
        result = result + "." ;
        Find( tree->right, result ) ; 
      } // if    
      else if ( tree->token.tType == LEFTP ) {
        result = result + "(" ;
        Find( tree->left, result ) ;
        Find( tree-> right, result ) ;
        result = result + ")" ;
        mspace = false ;
      } // if
      else {
        result = result + tree->token.tokenStr ;
      } // else
    } // else if
    else {
      Find( tree->left, result ) ;
      if ( tree->isDot )
        result = result + "." ;
      
      Find( tree->right, result ) ;
    } // else
  } // Find()
  
  bool IsPrimitive( string name ) {
    if ( name == "cons" || name == "list" || name == "quote" || name == "define" || name == "exit"
         || name == "car" || name == "cdr" || name == "atom?" || name == "pair?" || name == "lambda"
         || name == "list?" || name == "null?" || name == "integer?" || name == "real?" || name == "let"
         || name == "number?" ||  name == "string?" || name == "boolean?" || name == "symbol?"
         || name == "+" || name == "-" || name == "*" || name == "/" || name == "not" 
         || name == "and" || name == "or" || name == ">" || name == ">=" || name == "<"
         || name == "<=" || name == "=" || name == "string-append" || name == "string>?" 
         || name == "string<?" || name == "string=?" || name == "eqv?" || name == "equal?"
         || name == "begin" || name == "if" || name == "cond" || name == "clean-environment" )
      return true ; 
    else
      return false ;
  } // IsPrimitive()
  
  bool IsKnown( string name ) {
    if ( IsPrimitive( name ) )
      return true ;
    else {
      for ( int i = 0 ; i < gFuncDefine.size() ; i++ )
        if ( name == gFuncDefine.at( i ).name ) 
          return true ;
          
      return false ;
    } // else
  } // IsKnown()
  
  bool PureList( TreePtr head ) {
    TreePtr walk = head ;
    while ( walk != NULL ) {
      if ( walk->isDot )
        return false ;
      walk = walk->right ;
    } // while
    
    return true ;
  } // PureList()
  
  int FindFunc( string token, TreePtr & bound ) {
    for ( int i = 0 ; i < gFuncDefine.size() ; i++ ) {
      if ( token == gFuncDefine.at( i ).name ) {
        bound = gFuncDefine.at( i ).defined ;
        return i ;
      } // if 
    } // for
      
    return -1 ;  
  } // FindFunc()
  /*
  bool ChangeSymbol( TreePtr & head ) {
    TreePtr temp ;
    if ( head == NULL )
      return true ;
    else if ( head->exist && IsAtomNotSym( head->token.tType ) )
      return true ;
    else if ( head->exist && head->token.tType == SYMBOL ) {     
      if ( IsKnown( head->token.tokenStr ) || FindFunc( head->token.tokenStr, temp ) != -1 )
        return true ;
      else {
        TreePtr origin ;
        TreePtr get = Copy( FindBound( head, origin ), NULL ) ;
        if ( get == NULL ) {
          cout << "ERROR (unbound symbol) : " << head->token.tokenStr << "\n\n" ;
          return false ;
        } // if
        else {
          head = get ;
          return true ;
        } // else
      } // else
    } // else if
    else {
      if ( head->left != NULL && head->left->exist && 
           ( head->left->token.tokenStr == "lambda" 
             || head->left->token.tokenStr == "let"
             || head->left->token.tokenStr == "quote" ) )
        return true ;
      else {
        if ( ChangeSymbol( head->left ) && ChangeSymbol( head->right ) )
          return true ;
        else
          return false ;
      } // else
    } // else
  } // ChangeSymbol()
  */

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
  
  TreePtr FindBound( TreePtr head, int & position ) {
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
            return gUseFunc.at( j ).args.at( i ).defined ;
          } // if 
        } // for
        
      if ( j-1 >= 0 && gUseFunc.at( j ).type != LET ) 
        inLet = false ;
        
      if ( j == last && gUseFunc.at( j ).name == "not defined" )
        last-- ;
    } // for 
    
    
    for ( int i = 0 ; i < gDefineTree.size() ; i++ ) {
      if ( head->token.tokenStr == gDefineTree.at( i ).name ) {
        position = -1 ; 
        return gDefineTree.at( i ).defined ;
      } // if 
    } // for
    
    for ( int i = 0 ; i < gSystemSymbol.size() ; i++ ) {
      if ( head->token.tokenStr == gSystemSymbol.at( i ).name ) {
        position = -1 ;
        return gSystemSymbol.at( i ).defined ;
      } // if 
    } // for

    return NULL ;
  } // FindBound()
  
  TreePtr FindDefineBound( TreePtr head ) {    
    for ( int i = 0 ; i < gDefineTree.size() ; i++ ) {
      if ( head->token.tokenStr == gDefineTree.at( i ).name ) {
        return gDefineTree.at( i ).defined ;
      } // if 
    } // for
    
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
    
    for ( int i = 0 ; i < gDefineTree.size() ; i++ ) {
      if ( token == gDefineTree.at( i ).name ) {
        return Copy( gDefineTree.at( i ).defined, NULL ) ;
      } // if 
    } // for 
    
    for ( int i = 0 ; i < gSystemSymbol.size() ; i++ ) {
      if ( token == gSystemSymbol.at( i ).name ) {
        return Copy( gSystemSymbol.at( i ).defined, NULL ) ;
      } // if 
    } // for  
    
    return NULL ;
  } // FindBound()
  
  TreePtr Copy( TreePtr cur, TreePtr parent ) {
    if ( cur == NULL )
      return NULL ;
    
    TreePtr copy = new Tree() ;
    copy->exist = cur->exist ;
    copy->isDot = cur->isDot ;
    copy->token = cur->token ;
    copy->parent = parent ;
    copy->left = Copy( cur->left, cur ) ;
    copy->right = Copy( cur->right, cur ) ;
    return copy ; 
  } // Copy()
  
  int FindDefined( string name ) {
    for ( int i = 0 ; i < gDefineTree.size() ; i++ ) {
      if ( name == gDefineTree.at( i ).name )
        return i ;
    } // for
    
    return -1 ;
  } // FindDefined()
  
  int FindFuncDefined( Func func ) {
    for ( int i = 0 ; i < gFuncDefine.size() ; i++ ) {
      if ( func.name == gFuncDefine.at( i ).name )
        return i ;
    } // for
    
    return -1 ;
  } // FindFuncDefined()
  
  
  
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
    temp->isDot = false ;
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
  
  int IsFunc( TreePtr head, TreePtr & cur, bool copy ) {
    TreePtr bound ;
    if ( cur->left != NULL && cur->left->exist 
         && FindFunc( cur->left->token.tokenStr, bound ) != -1 ) {
      return 1 ;
    } // if
    else
      return 0 ;
  } // IsFunc()
  
  void Change( TreePtr & cur, string token ) {
    // change defined symbol to its binding
    if ( cur == NULL ) ;
    else if ( cur->exist && cur->token.tokenStr == token )
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
    else if ( cond->exist && cond->token.tType == NIL ) {
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
      
      if ( copy ) {
        correct = 1 ;
        return Copy( res, NULL ) ;
      } // if
      else {
        correct = 1 ;
        return res ;
      } // else 
    } // else
  } // Cond()
  
  TreePtr Evaluate( TreePtr head, TreePtr cur, string func, bool copy, 
                    vector <TreePtr> args ) {
    /*
    cout << "-------------------Evaluate--------------------\n" ;
    Print( cur, 0 ) ; 
    for ( int i = 0 ; i < gUseFunc.size() ; i++ ) {
      cout << "========================\n" << i << endl; 
      cout << gUseFunc.at( i ).name << endl ;
      Print( gUseFunc.at( i ).defined, 0 ) ;
    } // for
    cout << endl ;
    */
    
    if ( func == "cons" ) { 
      TreePtr temp = new Tree ;
      temp->exist = true ;
      temp->token.tokenStr = "(" ;
      temp->token.tType = LEFTP ;
      temp->left = NULL ;
      temp->right = NULL ;
      temp->isDot = true ;
      TreePtr s2 = args.at( 0 ) ;
      temp->left = s2 ;
      TreePtr s3 = args.at( 1 ) ;
      temp->right = s3 ;
      if ( temp->right->token.tokenStr == "nil" ) {
        temp->isDot = false ;
        temp->right->exist = false ;
      } // if
      else if ( temp->right->token.tType == LEFTP ) {
        temp->isDot = false ;
        temp->right->exist = false ;
      } // else if
 
      if ( copy )
        return Copy( temp, NULL ) ;
      else
        return temp ;
    } // if
    else if ( func == "quote" ) {
      if ( copy )
        return Copy( args.at( 0 ), NULL ) ;
      else
        return args.at( 0 ) ;
    } // else if
    else if ( func == "list" ) {
      TreePtr res = new Tree ;
      TreePtr walk = res ;
      res->exist = true ;
      res->token.tokenStr = "(" ;
      res->token.tType = LEFTP ;
      res->isDot = false ;
      res->left = NULL ;
      res->right = NULL ;
      for ( int i = 0 ; i < args.size() ; i++ ) {
        walk->left = args.at( i ) ;
        walk->right = new Tree ;
        walk = walk->right ;
        walk->exist = false ;
        walk->isDot = false ;
        walk->left = NULL ;
        walk->right = NULL ;
      } // for
      
      walk->token.tokenStr = "nil" ;
      walk->token.tType = NIL ;
      
      if ( copy )
        return Copy( res, NULL ) ;
      else
        return res ;
    } // else if
    else if ( func == "define" ) {
      if ( cur->right->left->token.tType == SYMBOL ) {
        int pos = FindDefined( cur->right->left->token.tokenStr ) ;
        TreePtr res = Eval( head, cur->right->right->left, copy ) ;
        if ( pos != -1 ) {
          if ( res != NULL ) {
            if ( res->token.tokenStr == "no return value" ) {
              gNoReturn = Copy( cur->right->right->left, NULL ) ;
              return Set( "no return value", SYMBOL ) ;
            } // if
            
            TreePtr defined = cur->right->right->left ;
            int find = IsFunc( head, defined, copy ) ;
            if ( find == 1 )
              defined = res ;
            else if ( find == 0 )
              Change( defined, cur->right->left->token.tokenStr ) ;
            gDefineTree.at( pos ).defined = Copy( defined, NULL ) ;
            cout << gDefineTree.at( pos ).name << " defined\n\n" ;
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
            int find = IsFunc( head, defined, copy ) ;
            if ( find == 1 )
              defined = res ;
            else
              Change( defined, cur->right->left->token.tokenStr ) ;
            temp.defined = Copy( defined, NULL ) ;
            gDefineTree.push_back( temp ) ;
            cout << temp.name << " defined\n\n" ; 
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
            if ( walk->left->exist && walk->left->token.tType != SYMBOL ) {
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
        TreePtr exp = cur->right->right ;
        while ( exp->right->token.tokenStr != "nil" )
          exp = exp->right ; 
        temp.defined = Copy( exp->left, NULL ) ;
        temp.type = FUNC ;
        
        int pos = FindDefined( temp.name ) ;
        if ( pos != -1 ) {
          TreePtr tempT = new Tree ;
          tempT->exist = true ;
          tempT->isDot = false ;
          tempT->left = NULL ;
          tempT->right = NULL ;
          tempT->parent = NULL ;
          tempT->token.tokenStr = "#<procedure " + temp.name + ">" ;
          tempT->token.tType = STRING ;
          gDefineTree.at( pos ).defined = Copy( tempT, NULL ) ;
        } // if 
        else
          MakeFuncDefine( temp.name ) ;
          
        pos = FindFuncDefined( temp ) ;
        if ( pos == -1 ) {
          cout << temp.name << " defined\n\n" ; 
          gFuncDefine.push_back( temp ) ;
          pos = gFuncDefine.size()-1 ;
        } // if   
        else {
          cout << temp.name << " defined\n\n" ;
          gFuncDefine.at( pos ) = temp ;
        } // else

        return NULL ;
      } // else
    } // else if
    else if ( func == "lambda" ) {
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
          else if ( get->exist && get->token.tokenStr == "no return value" ) {
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
      if ( copy )
        return Copy( result, NULL ) ;
      else
        return result ;
    } // else if
    else if ( func == "car" ) {
      TreePtr s = args.at( 0 ) ;
      if ( copy )
        return Copy( s->left, NULL ) ;
      else 
        return s->left ; 
    } // else if
    else if ( func == "cdr" ) {
      TreePtr s = args.at( 0 ) ;
      TreePtr temp ;
      if ( copy )
        temp = Copy( s->right, NULL ) ;
      else
        temp = s->right ;
      if ( s->right->exist && IsAtom( s->right->token.tType ) ) {
        return temp ;
      } // if        
      else {
        temp->exist = true ;
        temp->token.tokenStr = "(" ;
        temp->token.tType = LEFTP ;
        return temp ;
      } // else   
    } // else if
    else if ( func == "atom?" ) {
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && IsAtom( get->token.tType ) )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "pair?" ) {
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && get->token.tType == LEFTP )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "list?" ) {
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && get->token.tType == NIL )
        return Set( "#t", T ) ;
      else if ( get->exist && IsAtom( get->token.tType ) ) ;
      else if ( PureList( get ) )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "null?" ) {
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && get->token.tType == NIL )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "integer?" ) {
      bool isInt = false ;
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && get->token.tType == INT )
        return Set( "#t", T ) ;
      else if ( get->exist && get->token.tType == FLOAT ) {
        double num = get->token.num ;
        if ( num == ( int ) num )
          return Set( "#t", T ) ;
      } // else if
      
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "real?" || func == "number?" ) {
      bool isReal = false ;
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && IsNum( get->token.tType ) )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "string?" ) {
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && get->token.tType == STRING )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "boolean?" ) {
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && IsBool( get->token.tType ) )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "symbol?" ) {
      TreePtr get = args.at( 0 ) ;
      if ( get->exist && get->token.tType == SYMBOL )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "+" ) {
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
      res->exist = true ;
      res->isDot = false ;
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
    } // else if
    else if ( func == "-" ) {
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
      res->exist = true ;
      res->isDot = false ;
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
    } // else if
    else if ( func == "*" ) {
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
      res->exist = true ;
      res->isDot = false ;
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
    } // else if
    else if ( func == "/" ) {
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
      res->exist = true ;
      res->isDot = false ;
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
    } // else if
    else if ( func == "not" ) {
      bool isNot = false ;
      TreePtr get = args.at( 0 ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && get->token.tType == NIL )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == ">" ) {
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
          if ( floatNum <= cmp )
            is = false ;
          else
            floatNum = cmp ;
        } // else
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == ">=" ) {
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
          if ( floatNum < cmp )
            is = false ;
          else
            floatNum = cmp ;
        } // else
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "<" ) {
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
          if ( floatNum >= cmp )
            is = false ;
          else
            floatNum = cmp ;
        } // else
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "<=" ) {
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
          if ( floatNum > cmp )
            is = false ;
          else
            floatNum = cmp ;
        } // else
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "=" ) {
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
          if ( floatNum != cmp )
            is = false ;
          else
            floatNum = cmp ;
        } // else
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "string-append" ) {
      string str = "" ;
      bool first = true ;
      for ( int i = 0 ; i < args.size() ; i++ ) {
        TreePtr get = args.at( i ) ;
        if ( first ) {
          str = get->token.tokenStr ;
          first = false ;
        } // if
        else {
          str.erase( str.begin()+str.length()-1 ) ;
          string copy = get->token.tokenStr ;
          copy.erase( copy.begin() ) ;
          str = str + copy ;
        } // else
      } // for
      
      TreePtr res = new Tree ; 
      res->exist = true ;
      res->isDot = false ;
      res->left = NULL ;
      res->right = NULL ;
      res->token.tokenStr = str ;
      res->token.tType = STRING ;     
      return res ;
    } // else if
    else if ( func == "string>?" ) {
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
          if ( cmp <= copy )
            is = false ;
          else
            cmp = copy ;
        } // else 
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "string<?" ) {
      string cmp = "" ;
      bool is = true, first = true ;
      TreePtr temp = cur->right ;
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
          if ( cmp >= copy )
            is = false ;
          else
            cmp = copy ;
        } // else 
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "string=?" ) {
      string cmp = "" ;
      bool is = true, first = true ;
      TreePtr temp = cur->right ;
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
          if ( cmp != copy )
            is = false ;
          else
            cmp = copy ;
        } // else 
      } // for
      
      if ( is )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "eqv?" ) {
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
    } // else if
    else if ( func == "equal?" ) {
      TreePtr cmp1 = args.at( 0 ) ;
      TreePtr cmp2 = args.at( 1 ) ;
      if ( IsEqual( cmp1, cmp2 ) )
        return Set( "#t", T ) ;
      return Set( "nil", NIL ) ;
    } // else if
    else if ( func == "and" ) {
      TreePtr temp = cur->right ;
      TreePtr res ;
      while ( temp != NULL ) {
        if ( temp->token.tokenStr != "nil" ) {
          TreePtr get = Eval( head, temp->left, copy ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && get->token.tokenStr == "no return value" ) {
            throw MyException( get->token.tokenStr, temp->left, 12 ) ;
          } // else if
          else if ( get->exist && get->token.tokenStr == "nil" )
            if ( copy )
              return Copy( get, NULL ) ;
            else
              return get ;
          else
            res = get ;
        } // if 
        
        temp = temp->right ;
      } // while
      
      if ( res )
        return Copy( res, NULL ) ;
      else
        return res ;
    } // else if
    else if ( func == "or" ) {
      TreePtr temp = cur->right ;
      TreePtr res ;
      while ( temp != NULL ) {
        if ( temp->token.tokenStr != "nil" ) {
          TreePtr get = Eval( head, temp->left, copy ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && get->token.tokenStr == "no return value" ) {
            throw MyException( get->token.tokenStr, temp->left, 12 ) ;
          } // else if
          else if ( get->exist && get->token.tokenStr != "nil" )
            if ( copy )
              return Copy( get, NULL ) ;
            else
              return get ;
          else
            res = get ;
        } // if 
        
        temp = temp->right ;
      } // while
      
      if ( res )
        return Copy( res, NULL ) ;
      else
        return res ;
    } // else if
    else if ( func == "if" ) {
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
          else if ( copy )
            return Copy( isTrue, NULL ) ;
          else
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
          
          if ( copy )
            return Copy( isFalse, NULL ) ;
          else
            return isFalse ;
        } // else
      } // else if
    } // else if
    else if ( func == "cond" ) {
      TreePtr walk = cur->right ;
      int num = GetArgsNum( cur->right ) ;
      for ( int i = 0 ; i < num ; i++ ) {
        if ( i < num - 1 ) {
          int correct = -1 ;        // -1:error, 0:not correct, 1:correct
          TreePtr get = Cond( head, walk->left, copy, correct ) ;
          if ( correct == -1 )
            return NULL ;
          else if ( correct == 1 )
            return get ;
        } // if
        else {      // ordinary
          TreePtr ordinary = walk->left ;
          if ( ordinary->exist && ( ordinary->left->token.tokenStr == "else" 
                                    || ordinary->left->token.tType == T ) ) {
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
      
            if ( copy )
              return Copy( res, NULL ) ;
            else
              return res ;
          } // if
          else {
            TreePtr check = Eval( head, ordinary->left, copy ) ;
            if ( check == NULL ) 
              return NULL ;
            else if ( check->exist && check->token.tType == NIL ) {
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
            
              if ( copy )
                return Copy( res, NULL ) ;
              else
                return res ;
            } // else
          } // else
        } // else
        
        walk = walk->right ;
      } // for
    } // else if
    else if ( func == "let" ) {
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
          if ( res == NULL )
            return NULL ;
          else if ( res->token.tokenStr == "no return value" ) {
            throw MyException( res->token.tokenStr, gNoReturn, 8 ) ;
          } // else if
          
          store.defined = res ;
          temp.args.push_back( store ) ;
          temp.num++ ;
        } // if
        
        walk = walk->right ;
      } // while

      gUseFunc.push_back( temp ) ;
      walk = cur->right->right ;
      TreePtr res ;
      while ( walk != NULL ) {
        if ( walk->token.tokenStr != "nil" ) {
          TreePtr get = Eval( head, walk->left, copy ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->token.tokenStr == "no return value" ) {
            if ( walk->right->token.tokenStr == "nil" ) {
              gNoReturn  = Copy( cur, NULL ) ;
              gUseFunc.erase( gUseFunc.end() ) ;
              return Set( "no return value", SYMBOL ) ;
            } // if
          } // else if
          else
            res = get ;
        } // if 
        
        walk = walk->right ;
      } // while
      
      gUseFunc.erase( gUseFunc.end() ) ;
      if ( copy )
        return Copy( res, NULL ) ;
      else
        return res ;
    } // else if
    else if ( func == "begin" ) {
      TreePtr temp = cur->right ;
      TreePtr res ;
      while ( temp != NULL ) {
        if ( temp->token.tokenStr != "nil" ) {
          TreePtr get = Eval( head, temp->left, copy ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && get->token.tokenStr == "no return value" ) {
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

      if ( copy )
        return Copy( res, NULL ) ;
      else
        return res ;
    } // else if
    else if ( func == "clean-environment" ) {
      gDefineTree.clear() ;
      gFuncDefine.clear() ;
      gUseFunc.clear() ;
      cout << "environment cleaned\n\n" ;
      return NULL ;
    } // else if
    else {
      TreePtr isBound = NULL ;
      int pos = FindFunc( func, isBound ) ;
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
      
      gUseFunc.push_back( gFuncDefine.at( pos ) ) ;
      for ( int i = 0 ; i < store.size() ; i++ ) {
        gUseFunc.at( gUseFunc.size()-1 ).args.at( i ).defined = store.at( i ) ;
      } // for
      
      pos = gUseFunc.size()-1 ;
      TreePtr res = Eval( isBound, isBound, copy ) ;
      if ( res != NULL && res->token.tokenStr == "no return value" ) {
        gNoReturn = cur ;  
      } // if
      
      if ( copy )
        res = Copy( res, NULL ) ;
      gUseFunc.erase( gUseFunc.begin()+pos ) ;
      return res ;
    } // else
    
    return NULL ;
  } // Evaluate()
  
  TreePtr Eval( TreePtr head, TreePtr cur, bool copy ) {
    /*
    cout << "\n-------------------Eval--------------------\n" ;
    cout << gUseFunc.size() << " " << position << endl;
    Print( cur, 0 ) ; 
    for ( int i = 0 ; i < gUseFunc.size() ; i++ ) {
      cout << gUseFunc.at( i ).name << endl ;
    } // for
    cout << endl ;          
    */
    string token ;
    vector <TreePtr> args ;
    bool isArgs = false ;
    if ( cur->exist && IsAtomNotSym( cur->token.tType ) )
      if ( copy )
        return Copy( cur, NULL ) ;
      else
        return cur ;
    else if ( cur->exist && cur->token.tType == SYMBOL ) {
      int place = -1 ;
      TreePtr isBound = FindBound( cur, place ) ;
      if ( isBound == NULL ) {
        throw MyException( cur->token.tokenStr, cur, 1 ) ;
      } // if
      else {
        if ( place != -1 ) {
          if ( copy )
            return Copy( isBound, NULL ) ;
          else 
            return isBound ;
        } // if
        else {
          if ( copy )
            return Copy( Eval( isBound, isBound, true ), NULL ) ;
          else 
            return Eval( isBound, isBound, false ) ;
        } // else
      } // else  
    } // else if 
    else {
      if ( !PureList( cur ) ) {
        throw MyException( cur->token.tokenStr, cur, 2 ) ;
        return NULL ;
      } // if
      else if ( head->left->exist && IsAtomNotSym( cur->left->token.tType ) ) {
        throw MyException( cur->left->token.tokenStr, cur, 3 ) ;
      } // if
      else if ( cur->left->exist && ( cur->left->token.tType == SYMBOL 
                                      || cur->left->token.tokenStr == "quote" ) ) {
        int type = 0 ;
        token = cur->left->token.tokenStr ;
        if ( IsKnown( token ) ) {
          if ( head != cur && ( token == "clean-environment"
                                || token == "define" 
                                || token == "exit" ) ) {
            throw MyException( token, cur, 4 ) ;
          } // if
          else if ( token == "exit" ) {
            throw MyException( token, cur, 5 ) ;
          } // else if 
          else if ( token == "define" ) {
            if ( GetArgsNum( cur->right ) < 2 ) {
              throw MyException( token, cur, 6 ) ;
            } // if
            else if ( IsAtomNotSym( cur->right->left->token.tType ) ) {
              throw MyException( token, cur, 6 ) ;
            } // if
            else if ( cur->right->left->token.tType == SYMBOL ) {
              if ( !CheckArgsNum( cur, "define" ) ) {
                throw MyException( token, cur, 6 ) ;
              } // if
              else if ( IsPrimitive( cur->right->left->token.tokenStr ) ) {
                throw MyException( token, cur, 6 ) ;
              } // if
              
              TreePtr result = Evaluate( head, cur, token, copy, args ) ;
              if ( result != NULL )
                if ( result->token.tokenStr == "(" 
                     && result->left == NULL && result->right == NULL ) {
                  result->token.tokenStr = "nil" ;
                  result->token.tType = NIL ;
                } // if
                
              return result ;
            } // if
            else {                                         
              TreePtr result = Evaluate( head, cur, token, copy, args ) ;
              if ( result != NULL )
                if ( result->token.tokenStr == "(" 
                     && result->left == NULL && result->right == NULL ) {
                  result->token.tokenStr = "nil" ;
                  result->token.tType = NIL ;
                } // if
                
              return result ;
            } // else
          } // else if
          else if ( token == "cond" ) {
            if ( !CheckArgsNum( cur, "cond" ) ) {
              throw MyException( token, cur, 6 ) ;
            } // if
            else {
              TreePtr temp = cur->right ;
              int num = GetArgsNum( cur->right ) ;
              for ( int i = 0 ; i < num ; i++ ) {               
                if ( GetArgsNum( temp->left ) < 2 || !PureList( temp->left ) ) {
                  throw MyException( token, cur, 6 ) ;
                } // if
                
                temp = temp->right ;
              } // for
              
              TreePtr result = Evaluate( head, cur, token, copy, args ) ;
              if ( result != NULL )
                if ( result->token.tokenStr == "(" 
                     && result->left == NULL && result->right == NULL ) {
                  result->token.tokenStr = "nil" ;
                  result->token.tType = NIL ;
                } // if
            
              return result ;
            } // else
          } // else if
          else if ( token == "lambda" ) {
            if ( GetArgsNum( cur->right ) < 2 ) {
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
            
            TreePtr result = new Tree ;
            result->exist = true ;
            result->isDot = false ;
            result->left = NULL ;
            result->right = NULL ;
            result->parent = NULL ;
            result->token.tokenStr = "#<procedure lambda>" ;
            result->token.tType = SYMBOL ;
            return result ;
          } // else if
          else if ( token == "let" ) {
            if ( GetArgsNum( cur->right ) < 2 ) {
              throw MyException( token, cur, 6 ) ;
            } // if
            else {
              TreePtr temp = cur->right->left ;
              while ( temp != NULL ) {   
                if ( temp->token.tokenStr != "nil" ) {
                  if ( GetArgsNum( temp->left ) != 2 || !PureList( temp->left ) ) {
                    throw MyException( token, cur, 6 ) ;
                  } // if
                  else if ( temp->left->left->token.tType != SYMBOL ) {
                    throw MyException( token, cur, 6 ) ;
                  } // else if                  
                } // if  
                
                temp = temp->right ;
              } // while
              
              TreePtr result = Evaluate( head, cur, token, copy, args ) ;
              if ( result != NULL )
                if ( result->token.tokenStr == "(" 
                     && result->left == NULL && result->right == NULL ) {
                  result->token.tokenStr = "nil" ;
                  result->token.tType = NIL ;
                } // if
            
              return result ;
            } // else
          } // else if
          else if ( token == "if" || token == "and" || token == "or" || token == "begin" ) {
            if ( !CheckArgsNum( cur, token ) ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            TreePtr result = Evaluate( head, cur, token, copy, args ) ;
            if ( result != NULL )
              if ( result->token.tokenStr == "(" 
                   && result->left == NULL && result->right == NULL ) {
                result->token.tokenStr = "nil" ;
                result->token.tType = NIL ;
              } // if
            
            return result ;
          } // else if
          else {      // not define, cond
            if ( !CheckArgsNum( cur, token ) ) {
              throw MyException( token, cur, 5 ) ;
            } // if
            
            if ( token == "eqv?" )
              copy = false ;
            
            TreePtr bound = NULL ;
            if ( FindFunc( token, bound ) != -1 )
              isArgs = true ;
          } // else
        } // if
        else {
          int find = -1 ;
          TreePtr isBound = FindBound( cur->left, find ) ;
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
              
            TreePtr bound = NULL ;
            if ( FindFunc( token, bound ) != -1 )
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
          
        else if ( token == "cons" || token == "list" || token == "atom?" || token == "pair?" 
                  || token == "list?" || token == "null?" || token == "integer?" 
                  || token == "number?" || token == "string?" || token == "boolean?" 
                  || token == "symbol?" || token == "not" || token == "equal?" 
                  || token == "eqv?" || token == "real?" ) {
          if ( arg->exist && arg->token.tokenStr == "no return value" ) {
            throw MyException( token, gNoReturn, 9 ) ;
          } // if
        } // else if
        else if ( token == "car" || token == "cdr" ) {
          if ( arg->exist && arg->token.tokenStr == "no return value" ) {
            throw MyException( token, gNoReturn, 9 ) ;
          } // if
          else if ( arg->exist && IsAtom( arg->token.tType ) )
            type = false ;
        } // else if
        else if ( token == "+" || token == "-" || token == "*" || token == "/" || token == "=" 
                  || token == ">" || token == ">=" || token == "<" || token == "<=" ) {
          if ( arg->exist && arg->token.tokenStr == "no return value" ) {
            throw MyException( token, gNoReturn, 9 ) ;
          } // if
          else if ( arg->exist && !IsNum( arg->token.tType ) )
            type = false ;
        } // else if
        else if ( token == "string-append" || token == "string>?" || token == "string<?"
                  || token == "string=?" ) {
          if ( arg->exist && arg->token.tokenStr == "no return value" ) {
            throw MyException( token, gNoReturn, 9 ) ;
          } // if          
          else if ( arg->exist && arg->token.tType != STRING )
            type = false ;
        } // else if
        
        if ( !type ) {
          throw MyException( token, arg, 10 ) ;
        } // if
        else 
          args.push_back( arg ) ;
      } // if  
      
      temp = temp->right ;
    } // while
    
    TreePtr result = Evaluate( head, cur, token, copy, args ) ;
    if ( result != NULL )
      if ( result->token.tokenStr == "(" 
           && result->left == NULL && result->right == NULL ) {
        result->token.tokenStr = "nil" ;
        result->token.tType = NIL ;
      } // if
            
    return result ;
  } // Eval()
};

bool JumpChar( char ch ) {
  if ( gChar == '\t' || gChar == ' ' || gChar == '\n' )
    return true ;
  else if ( gChar == ')' || gChar == ';' || gChar == '\"' )
    return true ;
  else  
    return false ;
} // JumpChar()

bool ReadSExp( vector <TokenS> & expr, bool & exit, bool & end, bool & error, 
               bool & second, MakeTree & makeTree ) {
  Scanner scan ; 
  vector <TokenS> tokens, test ;
  TokenS tkn ;
  int paren = 0 ;
  int line, column, errorType ;
  bool isParen = false, quoteE = false ;
  bool skip_com = false, make = true ;
  bool quote = false, dQuote = false ;    // dQoute:
  bool jump = false, isEnd = false ;
  do {
    if ( paren > 0 )
      gParen = true ;
    else
      gParen = false ;
    
    skip_com = false ;
    if ( !second && !dQuote ) {
      if ( !scan.GetChar( line, column ) )
        end = true ;
    } // if
    else {
      gLine = gNextLine ;
      gColumn = gNextColumn ;
    } // else
    
    if ( !end ) {
      second = false ;
      make = scan.MakeToken( tokens, paren, quoteE, skip_com, line, column, second, quote, dQuote ) ;
      if ( paren > 0 )
        isParen = true ;
      else 
        isParen = false ;
    
      if ( !make ) 
        end = true ; 
      if ( end && isParen ) {
        isEnd = true ;
      } // if
         
      if ( tokens.size() > 0 && !quoteE && !isEnd )
        if ( !scan.Grammer( tokens, test, errorType, tkn, false ) ) { 
          jump = true ;
          second = false ;
        } // if  

      if ( skip_com && make ) {
        if ( !scan.Skip() )
          end = true ;
      } // if
    
      test.clear() ;
    } // if
    
  } while ( ( !JumpChar( gChar ) || isParen || skip_com || quote || dQuote ) 
            && !second && !end && !quoteE && !jump ) ; // while
  
  if ( end && gParen ) {
    expr.clear() ;
    tokens.clear() ;
    return true ;
  } // if

  if ( quoteE ) {
    cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " ;
    cout << gLine << " Column " << gColumn << endl << endl ;
    tokens.clear() ;
    scan.Skip() ;
    return true ;
  } // if
  else if ( tokens.size() > 0 ) {
    /*
    for ( int i = 0 ; i < tokens.size() ; i++ )
      cout << tokens.at( i ).tokenStr << " " ;
    cout << endl ; 
    */
    
    if ( scan.Grammer( tokens, expr, errorType, tkn, true ) ) {
      scan.Paren( expr ) ; 
      makeTree.Create( expr ) ;    
      tokens.clear() ;
    } // if
    else {
      if ( errorType == 1 || errorType == 3 ) {
        cout << "ERROR (unexpected token) : atom or '(' expected when token at Line " ;
        cout << tkn.line << " Column " << tkn.column << " is >>" << tkn.tokenStr << "<<\n\n" ;
        expr.clear() ;
        tokens.clear() ;
      } // if
      else if ( errorType == 2 ) {
        cout << "ERROR (unexpected token) : ')' expected when token at Line " ;
        cout << tkn.line << " Column " << tkn.column << " is >>" << tkn.tokenStr << "<<\n\n" ;
        expr.clear() ;
        tokens.clear() ;
      } // else if
      
      if ( !second )
        if ( !scan.Skip() ) {
          end = true ;
          exit = false ;
        } // if
          
        
      return true ;
    } // else

  } // if
  
  return false ;
} // ReadSExp()

void PrintSExp( vector <TokenS> expr, TreePtr res ) {  
  MakeTree tree ;
  MakeIns ins ;
  if ( res->token.tokenStr == "no return value" ) {
    cout << "ERROR (no return value) : " ;
    ins.Print( gNoReturn, 0 ) ;
    cout << endl ;
  } // if
  else {
    tree.Print( res, 0 ) ;
    if ( expr.size() > 0 )
      cout << endl ;
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
  bool exit = false, end = false, tokenError = false ;
  bool second = false, printArrow = true ;
  bool finalPrint = true ;
  int lastLine = 0 ;
  ins.Set() ;
  while ( !exit && !end ) {
    if ( printArrow ) {
      cout << "> " ;
      finalPrint = false ; 
    } // if  
    
    bool error = ReadSExp( expr, exit, end, tokenError, second, tree ) ;
    bool clear = true ;
    if ( expr.size() > 0 ) {
      TreePtr temp = tree.GetTree() ;
      if ( tree.IsExit() ) {
        end = false ;
        exit = true ;
      } // if
      
      if ( !error && !exit && !tokenError ) {
        try {
          TreePtr result = ins.Eval( temp, temp, true ) ;
          if ( result != NULL ) {
            PrintSExp( expr, result ) ;
            tree.Delete( result ) ; 
          } // if
        } // try
        catch( MyException & e ) {
          e.Error() ;
        } // catch
        
        
        gUseFunc.clear() ;
      } // if
      
      tree.Delete( temp ) ;
      // PrintSExp( expr, temp ) ;
    } // if
    
    if ( expr.size() > 0 ) {
      lastLine = gAbsLine ;
      gNextLine = 1 ;
      if ( gChar != ' ' && gChar != '\t' )
        gNextColumn = 1 ; 
      else
        gNextColumn = 2 ;
    } // if
    else {
      if ( lastLine == gAbsLine ) {
        gNextLine = 1 ;
        gNextColumn = 1 ; 
      } // if
    } // else
       
    finalPrint = false ;
    if ( expr.size() > 0 || error || tokenError )
      if ( end ) {
        printArrow = false ;
        finalPrint = true ;
      } // if
      else
        printArrow = true ; 
    else 
      printArrow = false ;     
      
    expr.clear() ;
    tree.Build() ;
  } // while()

  bool check = false ;
  if ( end && gParen )
    check = true ;
  if ( finalPrint && !check )
    cout << "> " ;
    
  if ( end )     
    cout << "ERROR (no more input) : END-OF-FILE encountered" ;
    
  cout << "\nThanks for using OurScheme!" ; 
} // main() 
