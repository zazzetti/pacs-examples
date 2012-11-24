/*
 * bound_cond.hpp
 *
 *  Created on: Nov 28, 2008
 *      Author: forma
 */

#ifndef BOUND_COND_HPP_
#define BOUND_COND_HPP_
#include <string>
#include <vector>
#include <iostream>
#include <functional>

namespace FEM {
  enum BCType {
    Dirichlet, Neumann, Robin, Generic, Other
  };
  
  
  //! This struct encapsulates the boundary conditions identifiers
  /*!
   *  Boundary condition identifiers are formed by a BCType entry, which select the type of
   *  boundary conditions, and a string, which gives a name of the BC within that type.
   *  For instance: BCId may contain Dirichlet and "Wall" which identifies the boundary condition
   *  "Wall" of being of Dirichlet type. We will not allow two BC with the same name, even if they
   *  are of different type
   *  The class is a template to allow different definitions of BCName. Indeed, in many implementations
   *  we require that BCName be just an int. The important thing is that a BCName should have an ordering
   *  since we may use it later as a key.
   */
  template <class BCName=std::string>
  struct BCId{
    //! To be able to know which type we are using for names
    typedef BCName name_t;
    //! The constructor takes a BCType  and a BCName
    BCId(BCType t, BCName n):type(t),name(n){};
    //! the type
    BCType type;
    //! the name
    BCName name;
  };

  //! Utility function which builds an ID
  template<class BCName>
  inline BCId make_BCId(BCType typ, BCName const & n){return BCId(typ,n);}

  //! Ordering relations for the IDs
  /*!
   * We define a the less<> functor orders the BCId in a lexicografic order
   * with the type first. In this way we assure tha BCs with the same type
   * are "adiacent" in a multiset container. We provide a partial specialization
   * of std::less<T>. We also define a std::equal_to<T> in the case we want
   * to use unordered sets. Yet we still require that both BCtype and BCName be
   * less-than comparable since equal_to<> is implemented in function of less<>.
   */
  template<class BCName>
  std::less<BCId<BCName> > {
    bool operator()(BCId<BCName>const & a; BCId<BCName>const & b)
    {
      if(!std::less<BCType>(a.type,b.type) && !std::less<BCType>(b.type,a.type)) //same type!
	return std::less<BCName>(a.name,b.name);
      else
	return std::less<BCType>(a.type,b.type);
    }
  };
  //! Equality operator.  
  template<class BCName>
  std::equal_to<BCId<BCName> > {
    bool operator()(BCId<BCName>const & a; BCId<BCName>const & b)
    {
      return !std::less<BCId<BCName> >(a,b) && !std::less<BCId<BCName> >(b,a);
    }
  };

  
  //! The type of the function  which imposes the bc
  typedef std::function<double (double const t, double const * coord)> BcFun;
  
  //! the zero function
  BCFun zerofun;

  //! A class for holding BConditions
  /*!This is a concrete base class to implement boundary conditions in a
   * finite element code. A boundary condition is identified by a type,
   * which is an enum and gives the code the indication on how the bc is applied,
   * a name, which is a string chosen by the user, for instance "wall".
   * A BCBase is uniquely identified by the type and the name.
   * A BCBase contains a vector of indices to "entities", which are tipically
   * the geometry entities to which the bc applies. For a Neumann and Robin
   * boundary contition it will be a list of faces/edges, for a Dirichlet boundary condition a
   * list of nodes. Since the BCBase will be stored in a set we need to make
   * those entities writable on constant object! So they are declared mutable.
   *
   * A function is recalled by the apply() method, and it implements
   * the boundary condition the function ha as argument the time and the
   * coordinate of a point
   */
  class BCBase {
  public:
    //! Constructor using string as names
    explicit  BCBase(BCType t=Dirichlet,
		     BCName n = std::string("Homogeneous"),
		     BcFun fun = zerofun):_entities(),_fun(fun),_id(t,n){};
    //! Constructor if the name is an integer
    explicit BCBase(BCType t,
		    int n,
		    BcFun fun = zerofun):_entities(),_fun(fun),_id(t,n){};
    //! It returns the identifier
    BCId get_Id() const {return _id;}
    //! Sets the identifier
    void set_Id(BCId const & id){_id = id;}
    //! Changes the function
    void set_fun(BcFun const f)const{_fun = f;}
    // Returns the name
    BCName name()const {return _id.name;}
    // Returns the type
    BCType type()const {return _id.type;}
    //! Applies boundary condition
    double apply(double const t, double const * coord) const {
      return _fun(t,coord);
    }
    //! It sets the entities to a new value
    void set_entities(std::vector<int> const & e) const;
    //! It returns the vector of entity index for any use (const version)
    std::vector<int> const & entities() const {return _entities;}
    void showMe(std::ostream & stream=std::cout) const;
    //! Predicates
    friend class isBCTypeEqual;
    friend class isBCNameEqual;
    //! Ordering relation for storing in a set/map
    friend bool operator <(BCBase const &,BCBase const &);
    friend bool operator ==(BCBase const &, BCBase const &);
  protected:
    //! The entities
    //! They must be mutable since they can be changed
    //! after the container has been set
    mutable std::vector<int> _entities;
    mutable BcFun _fun;
    BCId _id;
  };
  //! Ordering relations for the boundary conditions
  inline bool operator < (BCBase const & l, BCBase const & r){
    return l._id<r._id;}
  inline bool operator == (BCBase const & l, BCBase const & r)
  {return l._id==r._id;}
  
  //! Useful predicates
  class isBCTypeEqual{
  public:
    isBCTypeEqual(BCType t):_type(t){};
    bool operator() (const BCBase & b){return b._id.type==_type;}
  private:
    BCType _type;
  };
  
  class isBCNameEqual{
  public:
    isBCNameEqual(BCName n):_name(n){};
    bool operator() (const BCBase & b){return b._id.name==_name;}
  private:
    BCName _name;
  };
  //! Useful comparison operator
  /*!
   * It defines a strict weak ordering of BCBase according
   * to the stored type
   */
  bool compareOnType(BCBase const &, BCBase const &);
  
}
#endif /* BOUND_COND_HPP_ */