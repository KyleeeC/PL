# include <cstdio>
# include <cstdlib>
# include <cmath>
# include <iostream>
# include <string>
# include <vector>
# include <sstream>

using namespace std ; 

enum Token { SEXP = 40, ATOM = 10 } ;

enum TerminalType { SYMBOL = 1000, INT = 500, FLOAT = 400, STRING = 300, NIL = 200, T = 100, 
                    LEFTP = 50, RIGHTP = 40, QUOTE = 20, DOT = 10
};

char gChar = '\0', gLastChar = '\0' ;

int gLine = 1, gColumn = 1 ;
int gNextLine = 1, gNextColumn = 1 ;
int gAbsNextLine = 1, gAbsLine = 1 ;

struct TokenS {
  int line ;
  int column ;
  string tokenStr ;     // StyleCheckType string
  TerminalType tType ;
};

struct Tree {
  bool exist ;
  int level ;
  bool isDot ;
  TokenS token ;
  Tree * left ;
  Tree * right ;
  Tree * parent ;
};

typedef Tree * TreePtr ;

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
  
  bool IsQuote( vector <TokenS> tkn ) {
    for ( int i = 0 ; i < tkn.size() ; i++ ) {
      if ( tkn.at( i ).tType != QUOTE )
        return false ;
    } // for

    return true ;
  } // IsQuote()
  
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
  
  bool IsFloat( string & token ) {
      // DOT已經在之前判斷完 
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
    
    if ( findDot && findNum ) {
      if ( pos == token.length()-1 )
        token = token + "0" ;
        
      return true ;
    } // if
      
    return false ;  
    
  } // IsFloat()
  
  bool IsString( string token ) {
    if ( token.length() > 2 && token.at( 0 ) == '\"' && token.at( token.length()-1 ) == '\"' )
      return true ;
    else
      return false ;  
  } // IsString()
  
  string FloatNum( string token ) {
    float temp = atof( token.c_str() ) ;
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
  
  int SExp( vector <TokenS> tknS, int & pos, vector <TokenS> & expr, int start, int & type, TokenS & err ) {
    int test = Atom( tknS, pos, expr ) ;
    if ( test == 1 )
      return 1 ;
    else if ( pos >= tknS.size() )
      return 2 ;
    else if ( tknS.at( pos ).tokenStr == "(" ) {
      expr.push_back( tknS.at( pos ) ) ;
      start = pos+1 ;
      pos++ ;
      if ( pos >= tknS.size() )
        return 2 ;
      else {
        int make1 = SExp( tknS, pos, expr, start, type, err ) ;
        if ( make1 == 2 )
          return 2 ;
        else if ( make1 == 0 ) {
          err = tknS.at( pos ) ;
          return 0 ;
        } // else if
        else {
          start = pos ;
          if ( pos >= tknS.size() )
            return 2 ;
            
          int make2 = SExp( tknS, pos, expr, start, type, err ) ;
          while ( make2 == 1 ) {
            start = pos ;
            if ( pos >= tknS.size() )
              return 2 ; 
            
            make2 = SExp( tknS, pos, expr, start, type, err ) ;
          } // while 

          if ( make2 == 2 )
            return 2 ;
          else if ( type == 1 || type == 2 )
            return 0 ;
          else if ( pos == start+1 )
            return 0 ;
          
          pos = start ;         
          if ( pos >= tknS.size() )
            return 2 ;  
          else if ( tknS.at( pos ).tType == DOT ) {
            expr.push_back( tknS.at( pos ) ) ;
            pos++ ;
            start = pos ;
            if ( pos >= tknS.size() )
              return 2 ;
              
            int make3 = SExp( tknS, pos, expr, start, type, err ) ;
            if ( make3 == 0 ) {
              type = type ;
              return 0 ;
            } // if
            else if ( make3 == 2 )
              return 2 ;  
          } // if

          if ( pos >= tknS.size() )
            return 2 ;
          else if ( tknS.at( pos ).tokenStr == ")" ) {
            expr.push_back( tknS.at( pos ) ) ;
            pos++ ;
            return 1 ;
          } // if
          else {
            type = 2 ;
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
      if ( pos >= tknS.size() )
        return 2 ;
        
      int make4 = SExp( tknS, pos, expr, start, type, err ) ;
      return make4 ; 
    } // else if
    else {
      type = 3 ;
      err = tknS.at( pos ) ;
      return 0 ;
    } // else
      
  } // SExp()
  
  int Atom( vector <TokenS> tknS, int & pos, vector <TokenS> & expr ) {
    if ( pos >= tknS.size() ) 
      return 2 ;
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
    else if ( gLastChar != '\n' ) {
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

  bool Grammer( vector <TokenS> & tknS, vector <TokenS> & expr, int & type, TokenS & err ) {
    int pos = 0 ;
    int success = SExp( tknS, pos, expr, 0, type, err ) ; 
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
    bool special = false, comment = false ;
    bool change = false, jump = false ;
    bool notEOF = true ;
    while ( ( special || !IsSeparator( gChar ) ) && !jump && notEOF ) {
      if ( gChar == '(' && !special )
        paren++ ;
      if ( gChar == ')' && !special )
        paren-- ; 
      backup = tempS ;
      tempS = tempS + gChar ;    
      if ( gChar == '\"' && !special ) {
        if ( backup == "" ) {
          special = true ;
          tempS = "\"" ;
        } // if
        else {
          dQuote = true ;
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
        if ( !change && !GetChar( line, column ) )
          notEOF = false ;  
          
      if ( gChar == '\n' && special ) {
        special = false ; 
        quoteE = true ; 
      } // if
       
    }  // while
    
    // -----------------------------------finish get token--------------------
    
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
    else if ( tempS == "\'" ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = QUOTE ;
    } // else if
    else if ( tempS == "(" ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = LEFTP ;
    } // else if
    else if ( tempS == ")" ) {
      temp.line = lineSP ;
      temp.column = columnSP ;
      temp.tType = RIGHTP ;
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
      if ( ( !second && temp.tokenStr != "" && paren == 0 ) || quoteE ) {
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
      if ( ( !second && temp.tokenStr != "" && paren == 0 ) || quoteE ) {
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
      if ( ( !second && temp.tokenStr != "" && paren == 0 ) || quoteE ) {
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
      Find( tree-> right, str ) ; 
    } // else
  } // Find()
  
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
          cout << tree->token.tokenStr << endl ;
        } // if
        else {
          for ( int i = 0 ; i < spaceNum ; i++ )
            cout << "  " ;
      
          cout << tree->token.tokenStr << endl ;
        } // else
      } // else
    } // else if
    else {
      Print( tree->left, spaceNum ) ;
      Print( tree->right, spaceNum ) ;
    } // else
  } // Print()
  
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
      } // else
    } // if
  } // Delete()
  
public:
  MakeTree() {
    mhead = new Tree ;
    mhead->exist = false ;
    mhead->isDot = false ;
    mhead->left = NULL ;
    mhead->right = NULL ;
    mhead->parent = NULL ;
    mlevel = 0 ;
  } // MakeTree()
  
  void Create( vector <TokenS> expr ) {
    TreePtr parent = mhead ;
    TreePtr temp = mhead ;
    for ( int i = 0 ; i < expr.size() ; i++ ) {
      /*
      Print( mhead ) ;
      cout << "Current : " << expr.at( i ).tokenStr << expr.at( i ).tType << endl ;
      for ( int j = 0 ; j < expr.size() ; j++ ) {
        cout << expr.at( j ).tokenStr << " " ;
      }
      cout << endl ;
      cout << "Walk : " << endl;
      for ( TreePtr walk = temp ; walk != NULL ; walk = walk->parent ) {
        cout << "[" << walk->token.tokenStr << ","<< walk->level << "]" << endl ;
      }
      cout << endl << "============" << endl ;
      */
      
      if ( expr.at( i ).tType == LEFTP ) {
        if ( parent->right != NULL && temp == parent->right ) {
          temp->exist = false ;
          temp->level = mlevel ;
          temp->isDot = false ;
          temp->left = new Tree ;
          temp->right = NULL ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
        } // if
        
        mlevel++ ;
        temp->exist = true ;
        temp->level = mlevel ;
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
          if ( temp->exist && mlevel == temp->level 
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
        else if ( i < expr.size()-1 
                  && ( expr.at( i+1 ).tType == LEFTP || expr.at( i+1 ).tType == QUOTE ) ) {
          temp->right = new Tree ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->level = mlevel ;
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
          temp->level = mlevel ;
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
          temp->level = mlevel ;
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
        temp->level = mlevel ;
        temp->token = expr.at( i ) ;
        temp->isDot = false ;
        temp->left = new Tree ;
        temp->left->parent = temp ;
        temp->left->exist = true ;
        temp->left->isDot = false ;
        temp->left->level = mlevel ;
        temp->left->token.tokenStr = "quote" ;
        temp->left->token.tType = QUOTE ;
        temp->left->left = NULL ;
        temp->left->right = NULL ;
        temp->right = new Tree ;
        parent = temp ;
        temp->right->parent = parent ;
        temp = temp->right ;
        temp->exist = false ;
        temp->isDot = false ;
        temp->level = mlevel ;
        temp->left = NULL ;
        temp->right = NULL ;
        TokenS tempS ;
        tempS.tokenStr = ")" ;
        tempS.tType = RIGHTP ;
        if ( i < expr.size()-1 
             && ( expr.at( i+1 ).tType == LEFTP || expr.at( i+1 ).tType == QUOTE ) ) {  
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
        } // if
        else {
          expr.insert( expr.begin()+i+2, tempS ) ;
          temp->left = new Tree ;
          parent = temp ;
          temp->left->parent = parent ;
          temp = temp->left ;
          temp->isDot = false ;
          temp->level = mlevel ;
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
          if ( expr.at( i+1 ).tType != LEFTP ) {
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
          temp->level = mlevel ;
          temp->isDot = false ;
          temp->right = new Tree ;
          parent = temp ;
          temp->right->parent = parent ;
          temp = temp->right ;
          temp->exist = false ;
          temp->left = NULL ;
          temp->right = NULL ;
          
          expr.erase( expr.begin() + i ) ;   // erase DOT
          expr.erase( expr.begin() + i ) ;   // erase NIL
          i = i - 1 ;
        } // else if
        else {    // Is ATOM
          temp->exist = true ;
          temp->level = mlevel ;
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
        temp->level = mlevel ;
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
            temp->level = mlevel ;
            temp->left = NULL ;
            temp->right = NULL ; 
          } // if
          else {
            parent = temp ;
            temp = temp->right ;
          } // else
        } // if
        else if ( i < expr.size()-1 
                  && ( expr.at( i+1 ).tType == LEFTP || expr.at( i+1 ).tType == QUOTE ) ) {
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
          temp->level = mlevel ;
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
    TreePtr temp = mhead ;
    Delete( temp ) ;
    temp = NULL ;
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
  
}; // MakeTree

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
  bool quote = false, dQuote = false ;
  bool jump = false ;
  do {
    skip_com = false ;
    if ( !second && !dQuote ) {
      if ( !scan.GetChar( line, column ) ) {
        end = true ;
        exit = false ;
      } // if
    } // if
    else {
      gLine = gNextLine ;
      gColumn = gNextColumn ;
    } // else
    
    second = false ;
    
    make = scan.MakeToken( tokens, paren, quoteE, skip_com, line, column, second, quote, dQuote ) ;

    if ( paren > 0 )
      isParen = true ;
    else 
      isParen = false ;
    
    if ( !make ) {
      end = true ;
      exit = false ;
    } // if
    /*
    cout << endl << "TokenStr: " ;
    for ( int i = 0 ; i < tokens.size() ; i++ )
      cout << tokens.at( i ).tokenStr << " " ;
    cout << endl ; 
    */
    if ( tokens.size() > 0 && !quoteE )
      if ( !scan.Grammer( tokens, test, errorType, tkn ) ) { 
        jump = true ;
        second = false ;
      } // if  

    if ( skip_com ) {
      if ( !scan.Skip() ) {
        end = true ;
        exit = false ;
      } // if
    } // if
    
    test.clear() ;
  } while ( ( !JumpChar( gChar ) || isParen || skip_com || quote || dQuote ) 
            && !second && !end && !exit && !quoteE && !jump ) ; // while

  if ( quoteE ) {
    cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " ;
    cout << gLine << " Column " << gColumn << endl << endl ;
    tokens.clear() ;
    scan.Skip() ;
    return true ;
  } // if
  else if ( tokens.size() > 0 ) {
    if ( tokens.size() == 1 && tokens.at( 0 ).tType == QUOTE )
      tokens.clear() ;
    if ( scan.Grammer( tokens, expr, errorType, tkn ) ) {
      scan.Paren( expr ) ; 
      makeTree.Create( expr ) ;
      if ( makeTree.IsExit() ) {
        end = false ;
        exit = true ;
      } // if
    
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

void PrintSExp( vector <TokenS> expr, MakeTree tree ) {  
  tree.Print() ;  
  if ( expr.size() > 0 )
    cout << endl ;
} // PrintSExp() 

int main() {
  MakeTree tree ;
  int uTestNum = 0 ;
  char ch = '\0' ;
  scanf( "%d%c", &uTestNum, &ch ) ;
  cout << "Welcome to OurScheme!\n\n" ;
  vector <TokenS> expr ;
  bool exit = false, end = false, tokenError = false ;
  bool second = false, printArrow = true ;
  bool finalPrint = true ;
  int lastLine = 0 ;
  while ( !exit && !end ) {
    if ( printArrow ) {
      cout << "> " ;
      finalPrint = false ; 
    } // if  
    
    bool error = ReadSExp( expr, exit, end, tokenError, second, tree ) ;
    if ( !error && !exit && !tokenError )
      PrintSExp( expr, tree ) ;
        
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
    
    tree.Clear() ;
    expr.clear() ;
    tree.Build() ;
  } // while()

  if ( finalPrint )
    cout << "> " ;
    
  if ( end )     
    cout << "ERROR (no more input) : END-OF-FILE encountered" ;
    
  cout << "\nThanks for using OurScheme!" ;  
} // main() 
