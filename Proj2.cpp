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

vector <Define> gDefineTree ;

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
  bool jump = false ;
  do {
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
      if ( tokens.size() > 0 && !quoteE )
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
  tree.Print( res, 0 ) ;
  if ( expr.size() > 0 )
    cout << endl ;
} // PrintSExp() 


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
    else if ( token == "#<procedure exit>" )
      return "exit" ;
    else
      return token ; 
  } // ChangeType()
  
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
    MakeFuncDefine( "cons" ) ;
    MakeFuncDefine( "list" ) ;
    MakeFuncDefine( "quote" ) ;
    MakeFuncDefine( "define" ) ;
    MakeFuncDefine( "car" ) ;
    MakeFuncDefine( "cdr" ) ;
    MakeFuncDefine( "atom?" ) ;
    MakeFuncDefine( "pair?" ) ;
    MakeFuncDefine( "list?" ) ;
    MakeFuncDefine( "null?" ) ;
    MakeFuncDefine( "integer?" ) ;
    MakeFuncDefine( "real?" ) ;
    MakeFuncDefine( "number?" ) ;
    MakeFuncDefine( "string?" ) ;
    MakeFuncDefine( "boolean?" ) ;
    MakeFuncDefine( "symbol?" ) ;
    MakeFuncDefine( "+" ) ;
    MakeFuncDefine( "-" ) ;
    MakeFuncDefine( "*" ) ;
    MakeFuncDefine( "/" ) ;
    MakeFuncDefine( "not" ) ;
    MakeFuncDefine( "and" ) ;
    MakeFuncDefine( "or" ) ;
    MakeFuncDefine( ">" ) ;
    MakeFuncDefine( ">=" ) ;
    MakeFuncDefine( "<" ) ;
    MakeFuncDefine( "<=" ) ;
    MakeFuncDefine( "=" ) ;
    MakeFuncDefine( "string-append" ) ;
    MakeFuncDefine( "string>?" ) ;
    MakeFuncDefine( "string<?" ) ;
    MakeFuncDefine( "string=?" ) ;
    MakeFuncDefine( "eqv?" ) ;
    MakeFuncDefine( "equal?" ) ;
    MakeFuncDefine( "begin" ) ;
    MakeFuncDefine( "if" ) ;
    MakeFuncDefine( "cond" ) ;
    MakeFuncDefine( "clean-environment" ) ;
    MakeFuncDefine( "exit" ) ;
  } // Set()
  
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
    int num = GetArgsNum( cur ) ;   
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
    else
      return false ; 
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
  
  bool IsKnown( string name ) {
    if ( name == "cons" || name == "list" || name == "quote" || name == "define" || name == "exit"
         || name == "car" || name == "cdr" || name == "atom?" || name == "pair?"
         || name == "list?" || name == "null?" || name == "integer?" || name == "real?"
         || name == "number?" ||  name == "string?" || name == "boolean?" || name == "symbol?"
         || name == "+" || name == "-" || name == "*" || name == "/" || name == "not" 
         || name == "and" || name == "or" || name == ">" || name == ">=" || name == "<"
         || name == "<=" || name == "=" || name == "string-append" || name == "string>?" 
         || name == "string<?" || name == "string=?" || name == "eqv?" || name == "equal?"
         || name == "begin" || name == "if" || name == "cond" || name == "clean-environment" )
      return true ;    
    else 
      return false ;
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
  
  TreePtr FindBound( TreePtr head ) {
    for ( int i = 0 ; i < gDefineTree.size() ; i++ ) {
      if ( head->token.tokenStr == gDefineTree.at( i ).name ) {
        return gDefineTree.at( i ).defined ;
      } // if 
    } // for
    
    return NULL ;
  } // FindBound()
  
  TreePtr FindBound( string token ) {
    for ( int i = 0 ; i < gDefineTree.size() ; i++ ) {
      if ( token == gDefineTree.at( i ).name ) {
        return Copy( gDefineTree.at( i ).defined, NULL ) ;
      } // if 
    } // for
    
    return NULL ;
  } // FindBound()
  
  string LowerToUpper( string lower ) {
    string upper = "" ;
    for ( int i = 0 ; i < lower.size() ; i++ ) {
      char ch = toupper( lower.c_str()[i] ) ;
      upper = upper + ch ;
    } // for
    
    return upper ;
  } // LowerToUpper()
  
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
  
  string IntNum( int num ) {
    string token = "" ;
    stringstream sstream;
    sstream << num ;
    token = sstream.str() ;
    return token ;
  } // IntNum()
  
  string FloatNum( float temp ) {
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
  
  TreePtr Set( bool is ) {
    TreePtr temp = new Tree ;
    temp->exist = true ;
    temp->isDot = false ;
    temp->left = NULL ;
    temp->right = NULL ;
    if ( is ) {
      temp->token.tokenStr = "#t" ;
      temp->token.tType = T ;
    } // if
    else {
      temp->token.tokenStr = "nil" ;
      temp->token.tType = NIL ;
    } // else
    
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
  
  void Change( TreePtr & cur, string token ) {
    // change defined symbol to its binding
    if ( cur == NULL ) ;
    else if ( cur->exist && cur->token.tokenStr == token )
      cur = FindBound( token ) ;
    else {
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
  
  TreePtr Evaluate( TreePtr head, TreePtr cur, string func, bool copy ) {
    if ( func == "cons" ) {
      TreePtr temp = new Tree ;
      temp->exist = true ;
      temp->token.tokenStr = "(" ;
      temp->token.tType = LEFTP ;
      temp->left = NULL ;
      temp->right = NULL ;
      temp->isDot = true ;
      TreePtr s2 = Eval( head, cur->right->left, copy ) ;
      if ( s2 != NULL ) {
        temp->left = s2 ;
        TreePtr s3 = Eval( head, cur->right->right->left, copy ) ;
        if ( s3 != NULL ) {
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
        else
          return NULL ;
      } // if
      else
        return NULL ;
    } // if
    else if ( func == "quote" ) {
      if ( copy )
        return Copy( cur->right->left, NULL ) ;
      else
        return cur->right->left ;
    } // else if
    else if ( func == "list" ) {
      TreePtr temp = cur->right ;
      TreePtr res = new Tree ;
      TreePtr walk = res ;
      res->exist = true ;
      res->token.tokenStr = "(" ;
      res->token.tType = LEFTP ;
      res->isDot = false ;
      res->left = NULL ;
      res->right = NULL ;
      while ( temp != NULL && !temp->exist && temp->token.tokenStr != "nil" ) {
        TreePtr s = Eval( head, temp->left, copy ) ;
        if ( s != NULL ) {
          walk->left = s ;
          walk->right = new Tree ;
          walk = walk->right ;
          walk->exist = false ;
          walk->isDot = false ;
          walk->left = NULL ;
          walk->right = NULL ;
          temp = temp->right ;
        } // if
        else
          return NULL ;
      } // while
      
      walk->token.tokenStr = "nil" ;
      walk->token.tType = NIL ;
      
      if ( copy )
        return Copy( res, NULL ) ;
      else
        return res ;
    } // else if
    else if ( func == "define" ) {
      int pos = FindDefined( cur->right->left->token.tokenStr ) ;
      if ( pos != -1 ) {
        TreePtr defined = cur->right->right->left ;
        Change( defined, cur->right->left->token.tokenStr ) ;
        gDefineTree.at( pos ).defined = Copy( defined, NULL ) ;
        cout << gDefineTree.at( pos ).name << " defined\n\n" ;
        return NULL ;
      } // if
      else {
        Define temp ;
        temp.name = cur->right->left->token.tokenStr ;
        if ( Eval( head, cur->right->right->left, copy ) != NULL ) {
          TreePtr defined = cur->right->right->left ;
          Change( defined, cur->right->left->token.tokenStr ) ;
          temp.defined = Copy( defined, NULL ) ;
          gDefineTree.push_back( temp ) ;
          cout << temp.name << " defined\n\n" ; 
          return NULL ;
        } // if
        else
          return NULL ;
      } // else
    } // else if
    else if ( func == "car" ) {
      TreePtr s = Eval( head, cur->right->left, copy ) ;
      if ( s == NULL )
        return NULL ;
      else {
        if ( s->exist && IsAtom( s->token.tType ) ) {
          cout << "ERROR (car with incorrect argument type) : " << s->token.tokenStr << "\n\n" ;
          return NULL ;
        } // if
        
        if ( copy )
          return Copy( s->left, NULL ) ;
        else 
          return s->left ; 
      } // else
    } // else if
    else if ( func == "cdr" ) {
      TreePtr s = Eval( head, cur->right->left, copy ) ;
      if ( s == NULL )
        return NULL ;
      else {
        if ( s->exist && IsAtom( s->token.tType ) ) {
          cout << "ERROR (cdr with incorrect argument type) : " << s->token.tokenStr << "\n\n" ;
          return NULL ;
        } // if
        
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
      } // else
    } // else if
    else if ( func == "atom?" ) {
      bool isAtom = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && IsAtom( get->token.tType ) )
        isAtom = true ;
      return Set( isAtom ) ;
    } // else if
    else if ( func == "pair?" ) {
      bool isPair = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && get->token.tType == LEFTP )
        isPair = true ;
      return Set( isPair ) ;
    } // else if
    else if ( func == "list?" ) {
      bool isList = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && get->token.tType == NIL )
        isList = true ;
      else if ( get->exist && IsAtom( get->token.tType ) ) ;
      else if ( PureList( get ) )
        isList = true ;
      return Set( isList ) ;
    } // else if
    else if ( func == "null?" ) {
      bool isNull = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && get->token.tType == NIL )
        isNull = true ;
      return Set( isNull ) ;
    } // else if
    else if ( func == "integer?" ) {
      bool isInt = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      else if ( get->exist && get->token.tType == INT )
        isInt = true ;
      else if ( get->exist && get->token.tType == FLOAT ) {
        float num = atof( get->token.tokenStr.c_str() ) ;
        if ( num == ( int ) num )
          isInt = true ; 
      } // else if
      
      return Set( isInt ) ;
    } // else if
    else if ( func == "real?" || func == "number?" ) {
      bool isReal = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && IsNum( get->token.tType ) )
        isReal = true ;
      return Set( isReal ) ;
    } // else if
    else if ( func == "string?" ) {
      bool isStr = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && get->token.tType == STRING )
        isStr = true ;
      return Set( isStr ) ;
    } // else if
    else if ( func == "boolean?" ) {
      bool isBool = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && IsBool( get->token.tType ) )
        isBool = true ;
      return Set( isBool ) ;
    } // else if
    else if ( func == "symbol?" ) {
      bool isSym = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && get->token.tType == SYMBOL )
        isSym = true ;
      return Set( isSym ) ;
    } // else if
    else if ( func == "+" ) {
      float floatNum = 0.0 ;
      int intNum = 0 ;
      bool isInt = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( isInt && get->token.tType == INT ) {
              int tempNum = atoi( get->token.tokenStr.c_str() ) ;
              intNum += tempNum ; 
            } // if
            else if ( isInt && get->token.tType == FLOAT ) {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              floatNum = intNum ;
              floatNum = floatNum + tempNum ;
              isInt = false ;
            } // else if
            else {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              floatNum = floatNum + tempNum ;
            } // else
          } // else if
          else {
            cout << "ERROR (+ with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      TreePtr res = new Tree ; 
      res->exist = true ;
      res->isDot = false ;
      res->left = NULL ;
      res->right = NULL ;
      if ( !isInt ) {
        res->token.tokenStr = FloatNum( floatNum ) ;
        res->token.tType = FLOAT ;
      } // if
      else {
        res->token.tokenStr = IntNum( intNum ) ;
        res->token.tType = INT ;
      } // else
      
      return res ;
    } // else if
    else if ( func == "-" ) {
      float floatNum = 0.0 ;
      int intNum = 0 ;
      bool isInt = true, first = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( isInt && get->token.tType == INT ) {
              int tempNum = atoi( get->token.tokenStr.c_str() ) ;
              if ( first )
                intNum = tempNum ;
              else
                intNum = intNum - tempNum ; 
            } // if
            else if ( isInt && get->token.tType == FLOAT ) {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              floatNum = intNum ;
              if ( first )
                floatNum = tempNum ;
              else
                floatNum = floatNum - tempNum ;
                
              isInt = false ;
            } // else if
            else {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              if ( first )
                floatNum = tempNum ;
              else
                floatNum = floatNum - tempNum ;
            } // else
            
            first = false ;
          } // else if
          else {
            cout << "ERROR (- with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      TreePtr res = new Tree ; 
      res->exist = true ;
      res->isDot = false ;
      res->left = NULL ;
      res->right = NULL ;
      if ( !isInt ) {
        res->token.tokenStr = FloatNum( floatNum ) ;
        res->token.tType = FLOAT ;
      } // if
      else {
        res->token.tokenStr = IntNum( intNum ) ;
        res->token.tType = INT ;
      } // else
      
      return res ;
    } // else if
    else if ( func == "*" ) {
      float floatNum = 1.0 ;
      int intNum = 1 ;
      bool isInt = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( isInt && get->token.tType == INT ) {
              int tempNum = atoi( get->token.tokenStr.c_str() ) ;
              intNum = intNum * tempNum ; 
            } // if
            else if ( isInt && get->token.tType == FLOAT ) {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              floatNum = intNum ;
              floatNum = floatNum * tempNum ;
              isInt = false ;
            } // else if
            else {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              floatNum = floatNum * tempNum ;
            } // else
          } // else if
          else {
            cout << "ERROR (* with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      TreePtr res = new Tree ; 
      res->exist = true ;
      res->isDot = false ;
      res->left = NULL ;
      res->right = NULL ;
      if ( !isInt ) {
        res->token.tokenStr = FloatNum( floatNum ) ;
        res->token.tType = FLOAT ;
      } // if
      else {
        res->token.tokenStr = IntNum( intNum ) ;
        res->token.tType = INT ;
      } // else
      
      return res ;
    } // else if
    else if ( func == "/" ) {
      float floatNum = 1.0 ;
      int intNum = 1 ;
      bool isInt = true, first = true ;
      TreePtr temp = cur->right ;
      for ( temp = cur->right ; temp != NULL ; temp = temp->right ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( isInt && get->token.tType == INT ) {
              int tempNum = atoi( get->token.tokenStr.c_str() ) ;
              if ( first )
                intNum = tempNum ;
              else {
                if ( tempNum == 0 ) {
                  cout << "ERROR (division by zero) : /\n\n" ;
                  return NULL ;
                } // if
                
                intNum = intNum / tempNum ; 
              } // else
            } // if
            else if ( isInt && get->token.tType == FLOAT ) {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              floatNum = intNum ;
              isInt = false ;
              if ( first )
                floatNum = tempNum ;
              else {
                if ( tempNum == 0 ) {
                  cout << "ERROR (division by zero) : /\n\n" ;
                  return NULL ;
                } // if
                
                floatNum = floatNum / tempNum ;
              } // else  
            } // else if
            else {
              float tempNum = atof( get->token.tokenStr.c_str() ) ;
              if ( first )
                floatNum = tempNum ;
              else {
                if ( tempNum == 0 ) {
                  cout << "ERROR (division by zero) : /\n\n" ;
                  return NULL ;
                } // if
                
                floatNum = floatNum / tempNum ;
              } // else    
            } // else
            
            first = false ;
          } // else if
          else {
            cout << "ERROR (/ with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n\n" ;
            return NULL ;
          } // else
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
      } // if
      else {
        res->token.tokenStr = IntNum( intNum ) ;
        res->token.tType = INT ;
      } // else
      
      return res ;
    } // else if
    else if ( func == "not" ) {
      bool isNot = false ;
      TreePtr get = Eval( head, cur->right->left, true ) ;
      if ( get == NULL )
        return NULL ;
      if ( get->exist && get->token.tType == NIL )
        isNot = true ;
      return Set( isNot ) ;
    } // else if
    else if ( func == ">" ) {
      float floatNum = 0.0 ;
      bool compare = false, is = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( !compare ) {
              floatNum = atof( get->token.tokenStr.c_str() ) ;
              compare = true ;
            } // if
            else {
              float cmp = atof( get->token.tokenStr.c_str() ) ;
              if ( floatNum <= cmp )
                is = false ;
              else
                floatNum = cmp ;
            } // else
          } // else if
          else {
            cout << "ERROR (> with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == ">=" ) {
      float floatNum = 0.0 ;
      bool compare = false, is = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( !compare ) {
              floatNum = atof( get->token.tokenStr.c_str() ) ;
              compare = true ;
            } // if
            else {
              float cmp = atof( get->token.tokenStr.c_str() ) ;
              if ( floatNum < cmp )
                is = false ;
              else
                floatNum = cmp ;
            } // else
          } // else if
          else {
            cout << "ERROR (>= with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == "<" ) {
      float floatNum = 0.0 ;
      bool compare = false, is = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( !compare ) {
              floatNum = atof( get->token.tokenStr.c_str() ) ;
              compare = true ;
            } // if
            else {
              float cmp = atof( get->token.tokenStr.c_str() ) ;
              if ( floatNum >= cmp )
                is = false ;
              else
                floatNum = cmp ;
            } // else
          } // else if
          else {
            cout << "ERROR (< with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == "<=" ) {
      float floatNum = 0.0 ;
      bool compare = false, is = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( !compare ) {
              floatNum = atof( get->token.tokenStr.c_str() ) ;
              compare = true ;
            } // if
            else {
              float cmp = atof( get->token.tokenStr.c_str() ) ;
              if ( floatNum > cmp )
                is = false ;
              else
                floatNum = cmp ;
            } // else
          } // else if
          else {
            cout << "ERROR (<= with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == "=" ) {
      float floatNum = 0.0 ;
      bool compare = false, is = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && IsNum( get->token.tType ) ) {
            if ( !compare ) {
              floatNum = atof( get->token.tokenStr.c_str() ) ;
              compare = true ;
            } // if
            else {
              float cmp = atof( get->token.tokenStr.c_str() ) ;
              if ( floatNum != cmp )
                is = false ;
              else
                floatNum = cmp ;
            } // else
          } // else if
          else {
            cout << "ERROR (= with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == "string-append" ) {
      string str = "" ;
      bool first = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && get->token.tType == STRING ) {
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
          } // else if
          else {
            cout << "ERROR (string-append with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
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
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && get->token.tType == STRING ) {
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
          } // else if
          else {
            cout << "ERROR (string>? with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == "string<?" ) {
      string cmp = "" ;
      bool is = true, first = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && get->token.tType == STRING ) {
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
          } // else if
          else {
            cout << "ERROR (string<? with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == "string=?" ) {
      string cmp = "" ;
      bool is = true, first = true ;
      TreePtr temp = cur->right ;
      while ( temp != NULL ) {
        if ( temp->token.tType == NIL && temp->right == NULL ) ;
        else {
          TreePtr get = Eval( head, temp->left, true ) ;
          if ( get == NULL )
            return NULL ;
          else if ( get->exist && get->token.tType == STRING ) {
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
          } // else if
          else {
            cout << "ERROR (string=? with incorrect argument type) : " ;
            Print( get, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else
        } // else  
        
        temp = temp->right ;
      } // while
      
      return Set( is ) ;
    } // else if
    else if ( func == "eqv?" ) {
      if ( IsAtomNotSym( cur->right->left->token.tType ) 
           && IsAtomNotSym( cur->right->right->left->token.tType ) ) {
        if ( cur->right->left->token.tokenStr == cur->right->right->left->token.tokenStr
             && cur->right->left->token.tType != STRING )
          return Set( true ) ; 
        else
          return Set( false ) ;
      } // if
      
      TreePtr cmp1 = Eval( head, cur->right->left, false ) ;
      if ( cmp1 != NULL ) {
        TreePtr cmp2 = Eval( head, cur->right->right->left, false ) ;
        if ( cmp2 != NULL ) {
          /*
          if ( IsAtom( cmp1->token.tType ) && IsAtom( cmp2->token.tType ) )
            if ( cmp1->token.tokenStr == cmp2->token.tokenStr )
              return Set( true ) ; 
            else
              return Set( false ) ; 
          */
          
          if ( cmp1 == cmp2 )
            return Set( true ) ;
          else
            return Set( false ) ;
        } // if
        else
          return NULL ;
      } // if
      else
        return NULL ;
    } // else if
    else if ( func == "equal?" ) {
      TreePtr cmp1 = Eval( head, cur->right->left, true ) ;
      if ( cmp1 != NULL ) {
        TreePtr cmp2 = Eval( head, cur->right->right->left, true ) ;
        if ( cmp2 != NULL ) {
          return Set( IsEqual( cmp1, cmp2 ) ) ;
        } // if
        else
          return NULL ;
      } // if
      else
        return NULL ;
    } // else if
    else if ( func == "and" ) {
      TreePtr temp = cur->right ;
      TreePtr res ;
      while ( temp != NULL ) {
        if ( temp->token.tokenStr != "nil" ) {
          TreePtr get = Eval( head, temp->left, copy ) ;
          if ( get == NULL )
            return NULL ;
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
      else {
        int num = GetArgsNum( cur->right ) ;  
        if ( cond->token.tType != NIL ) {
          TreePtr isTrue = Eval( head, cur->right->right->left, copy ) ;
          if ( isTrue == NULL )
            return NULL ;
          else if ( copy )
            return Copy( isTrue, NULL ) ;
          else
            return isTrue ;
        } // if     
        else {
          TreePtr isFalse = NULL ;
          if ( num == 2 ) {
            cout << "ERROR (no return value) : " ;
            Print( cur, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // if
          
          isFalse = Eval( head, cur->right->right->right->left, copy ) ;
          if ( isFalse == NULL )
            return NULL ;
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
              cout << "ERROR (no return value) : " ;
              Print( cur, 0 ) ;
              cout << endl ;
              return NULL ;
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
    else if ( func == "begin" ) {
      TreePtr walk = cur->right ;
      TreePtr res ;
      while ( walk != NULL ) {
        if ( walk->token.tokenStr != "nil" ) {
          TreePtr get = Eval( head, walk->left, copy ) ;
          if ( get == NULL )
            return NULL ;
          else
            res = get ;
        } // if 
        
        walk = walk->right ;
      } // while
      
      if ( copy )
        return Copy( res, NULL ) ;
      else
        return res ;
    } // else if
    else if ( func == "clean-environment" ) {
      gDefineTree.clear() ;
      Set() ;
      cout << "environment cleaned\n\n" ;
      return NULL ;
    } // else if
    
    return NULL ;
  } // Evaluate()
  
  TreePtr Eval( TreePtr head, TreePtr cur, bool copy ) {
    if ( cur->exist && IsAtomNotSym( cur->token.tType ) )
      if ( copy )
        return Copy( cur, NULL ) ;
      else
        return cur ;
    else if ( cur->exist && cur->token.tType == SYMBOL ) {
      int num = 0 ;
      TreePtr isBound = FindBound( cur ) ;
      if ( isBound == NULL ) {
        cout << "ERROR (unbound symbol) : " << cur->token.tokenStr << "\n\n" ;
        return NULL ;
      } // if
      else {
        if ( copy )
          return Copy( Eval( isBound, isBound, true ), NULL ) ;
        else  {
          return Eval( isBound, isBound, false ) ;
        } // else        
      } // else  
    } // else if 
    else {
      if ( !PureList( cur ) ) {
        cout << "ERROR (non-list) : " ;
        Print( cur, 0 ) ;
        cout << endl ;
        return NULL ;
      } // if
      else if ( head->left->exist && IsAtomNotSym( cur->left->token.tType ) ) {
        cout << "ERROR (attempt to apply non-function) : " << cur->left->token.tokenStr << "\n\n" ;
        return NULL ;
      } // if
      else if ( cur->left->exist && ( cur->left->token.tType == SYMBOL 
                                      || cur->left->token.tokenStr == "quote" ) ) {
        if ( IsKnown( cur->left->token.tokenStr ) ) {
          if ( head != cur && ( cur->left->token.tokenStr == "clean-environment"
                                || cur->left->token.tokenStr == "define" 
                                || cur->left->token.tokenStr == "exit" ) ) {
            cout << "ERROR (level of " << LowerToUpper( cur->left->token.tokenStr ) << ")\n\n" ;
            return NULL ;
          } // if
          else if ( cur->left->token.tokenStr == "exit" ) {
            cout << "ERROR (incorrect number of arguments) : exit\n\n" ;
            return NULL ;
          } // else if 
          else if ( cur->left->token.tokenStr == "define" ) {
            if ( !CheckArgsNum( cur->right, "define" ) 
                 || cur->right->left->token.tType != SYMBOL
                 || IsKnown( cur->right->left->token.tokenStr ) ) {
              cout << "ERROR (DEFINE format) : " ;
              Print( head, 0 ) ;
              cout << endl ;
              return NULL ;
            } // if
              
            TreePtr result = Evaluate( head, cur, cur->left->token.tokenStr, copy ) ;
            if ( result != NULL )
              if ( result->token.tokenStr == "(" 
                   && result->left == NULL && result->right == NULL ) {
                result->token.tokenStr = "nil" ;
                result->token.tType = NIL ;
              } // if
            
            return result ;
          } // else if
          else if ( cur->left->token.tokenStr == "cond" ) {
            if ( !CheckArgsNum( cur->right, "cond" ) ) {
              cout << "ERROR (COND format) : " ;
              Print( head, 0 ) ;
              cout << endl ;
              return NULL ;
            } // if
            else {
              TreePtr temp = cur->right ;
              int num = GetArgsNum( cur->right ) ;
              for ( int i = 0 ; i < num ; i++ ) {               
                if ( GetArgsNum( temp->left ) < 2 || !PureList( temp->left ) ) {
                  cout << "ERROR (COND format) : " ;
                  Print( head, 0 ) ;
                  cout << endl ;
                  return NULL ;
                } // if
                
                temp = temp->right ;
              } // for
              
              TreePtr result = Evaluate( head, cur, cur->left->token.tokenStr, copy ) ;
              if ( result != NULL )
                if ( result->token.tokenStr == "(" 
                     && result->left == NULL && result->right == NULL ) {
                  result->token.tokenStr = "nil" ;
                  result->token.tType = NIL ;
                } // if
            
              return result ;
            } // else
          } // else if
          else {      // not define, if, and, or
            if ( !CheckArgsNum( cur->right, cur->left->token.tokenStr ) ) {
              cout << "ERROR (incorrect number of arguments) : " << cur->left->token.tokenStr << "\n\n" ;
              return NULL ;
            } // if
            
            TreePtr result = Evaluate( head, cur, cur->left->token.tokenStr, copy ) ;
            if ( result != NULL )
              if ( result->token.tokenStr == "(" 
                   && result->left == NULL && result->right == NULL ) {
                result->token.tokenStr = "nil" ;
                result->token.tType = NIL ;
              } // if
            
            return result ;
          } // else
        } // if
        else {
          TreePtr isBound = FindBound( cur->left ) ;
          if ( isBound == NULL ) {
            cout << "ERROR (unbound symbol) : " << cur->left->token.tokenStr << "\n\n" ;
            return NULL;
          } // if
          else {
            TreePtr get = Eval( head, isBound, copy ) ;
            if ( get == NULL )
              return NULL ;
              
            string token = ChangeType( get->token.tokenStr ) ;
            if ( IsKnown( token ) ) {
              if ( head != cur && ( token == "clean-environment" || token == "define" 
                                    || token == "exit" ) ) {
                cout << "ERROR (level of " << LowerToUpper( token ) << ")\n\n" ;
                return NULL ;
              } // if
              else if ( token == "exit" ) {
                cout << "ERROR (incorrect number of arguments) : exit\n\n" ;
                return NULL ;
              } // else if 
              else if ( token == "define" ) {
                if ( !CheckArgsNum( cur->right, "define" ) 
                     || cur->right->left->token.tType != SYMBOL
                     || IsKnown( cur->right->left->token.tokenStr ) ) {
                  cout << "ERROR (DEFINE format) : " ;
                  Print( head, 0 ) ;
                  cout << endl ;
                  return NULL ;
                } // if
              
                TreePtr result = Evaluate( head, cur, token, copy ) ;
                if ( result != NULL )
                  if ( result->token.tokenStr == "(" 
                       && result->left == NULL && result->right == NULL ) {
                    result->token.tokenStr = "nil" ;
                    result->token.tType = NIL ;
                  } // if
            
                return result ;
              } // else if
              else if ( token == "cond" ) {
                if ( !CheckArgsNum( cur->right, "cond" ) ) {
                  cout << "ERROR (COND format) : " ;
                  Print( head, 0 ) ;
                  cout << endl ;
                  return NULL ;
                } // if
                else {
                  TreePtr temp = cur->right ;
                  int num = GetArgsNum( cur->right ) ;
                  for ( int i = 0 ; i < num ; i++ ) {               
                    if ( GetArgsNum( temp->left ) < 2 || !PureList( temp->left ) ) {
                      cout << "ERROR (COND format) : " ;
                      Print( head, 0 ) ;
                      cout << endl ;
                      return NULL ;
                    } // if
                
                    temp = temp->right ;
                  } // for
              
                  TreePtr result = Evaluate( head, cur, token, copy ) ;
                  if ( result != NULL )
                    if ( result->token.tokenStr == "(" 
                         && result->left == NULL && result->right == NULL ) {
                      result->token.tokenStr = "nil" ;
                      result->token.tType = NIL ;
                    } // if
            
                  return result ;
                } // else
              } // else if
              else {      // not define, if, and, or
                if ( !CheckArgsNum( cur->right, token ) ) {
                  cout << "ERROR (incorrect number of arguments) : " << token << "\n\n" ;
                  return NULL ;
                } // if
            
                TreePtr result = Evaluate( head, cur, token, copy ) ;
                if ( result != NULL )
                  if ( result->token.tokenStr == "(" 
                       && result->left == NULL && result->right == NULL ) {
                    result->token.tokenStr = "nil" ;
                    result->token.tType = NIL ;
                  } // if
            
                return result ;
              } // else
            } // if
            else {
              cout << "ERROR (attempt to apply non-function) : " ;
              Print( isBound, 0 ) ;
              cout << "\n" ;
              return NULL;
            } // else
          } // else
        } // else
      } // else if 
      else {
        TreePtr noError = Eval( head, cur->left, copy ) ;
        if ( noError != NULL ) {
          string token = ChangeType( noError->token.tokenStr ) ; 
          if ( IsKnown( token ) ) {
            if ( head != cur && ( token == "clean-environment" || token == "define" 
                                  || token == "exit" ) ) {
              cout << "ERROR (level of " << LowerToUpper( token ) << ")\n\n" ;
              return NULL ;
            } // if
            else if ( token == "exit" ) {
              cout << "ERROR (incorrect number of arguments) : exit\n\n" ;
              return NULL ;
            } // else if 
            else if ( token == "define" ) {
              if ( !CheckArgsNum( cur->right, "define" ) 
                   || cur->right->left->token.tType != SYMBOL
                   || IsKnown( cur->right->left->token.tokenStr ) ) {
                cout << "ERROR (DEFINE format) : " ;
                Print( head, 0 ) ;
                cout << endl ;
                return NULL ;
              } // if
              
              TreePtr result = Evaluate( head, cur, token, copy ) ;
              if ( result != NULL )
                if ( result->token.tokenStr == "(" 
                     && result->left == NULL && result->right == NULL ) {
                  result->token.tokenStr = "nil" ;
                  result->token.tType = NIL ;
                } // if
            
              return result ;
            } // else if
            else if ( token == "cond" ) {
              if ( !CheckArgsNum( cur->right, "cond" ) ) {
                cout << "ERROR (COND format) : " ;
                Print( head, 0 ) ;
                cout << endl ;
                return NULL ;
              } // if
              else {
                TreePtr temp = cur->right ;
                int num = GetArgsNum( cur->right ) ;
                for ( int i = 0 ; i < num ; i++ ) {               
                  if ( GetArgsNum( temp->left ) < 2 || !PureList( temp->left ) ) {
                    cout << "ERROR (COND format) : " ;
                    Print( head, 0 ) ;
                    cout << endl ;
                    return NULL ;
                  } // if
                
                  temp = temp->right ;
                } // for
              
                TreePtr result = Evaluate( head, cur, token, copy ) ;
                if ( result != NULL )
                  if ( result->token.tokenStr == "(" 
                       && result->left == NULL && result->right == NULL ) {
                    result->token.tokenStr = "nil" ;
                    result->token.tType = NIL ;
                  } // if
            
                return result ;
              } // else
            } // else if
            else {      // not define, if, and, or
              if ( !CheckArgsNum( cur->right, token ) ) {
                cout << "ERROR (incorrect number of arguments) : " << token << "\n\n" ;
                return NULL ;
              } // if
            
              TreePtr result = Evaluate( head, cur, token, copy ) ;
              if ( result != NULL )
                if ( result->token.tokenStr == "(" 
                     && result->left == NULL && result->right == NULL ) {
                  result->token.tokenStr = "nil" ;
                  result->token.tType = NIL ;
                } // if
            
              return result ;
            } // else
          } // if
          else {
            cout << "ERROR (attempt to apply non-function) : " ;
            Print( noError, 0 ) ;
            cout << "\n" ;
            return NULL ;
          } // else if
        } // if
        else
          return NULL ;
      } // else
    } // else
  } // Eval()
};

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
        TreePtr result = ins.Eval( temp, temp, true ) ;
        if ( result != NULL ) {
          PrintSExp( expr, result ) ;
          tree.Delete( result ) ;
        } // if
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

  if ( finalPrint )
    cout << "> " ;
    
  if ( end )     
    cout << "ERROR (no more input) : END-OF-FILE encountered" ;
    
  cout << "\nThanks for using OurScheme!" ; 
} // main() 
