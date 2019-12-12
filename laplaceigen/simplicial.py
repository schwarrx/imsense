# Simplicial Complex Library (oriented and unoriented)
# Copyright (R) 2019 PARC - Project CyPhy (DARPA AIRA)
# Author: Morad Behandish

import numpy as np
import scipy.sparse as sparse
import networkx as nx

from networkx.drawing.nx_agraph import to_agraph, write_dot

from abc import ABC, abstractmethod
from itertools import combinations
import warnings
import json


def SignChar(number):
    """
    *** Pre-conditions:
    - [Not Asserted] Assume that input is comparable to integer zero.
    """
    if number > 0: return '+'
    if number < 0: return '–'
    return ""


def CreateHashKey(*args):
    """
    *** Pre-conditions:
    - [Not Asserted] Assume that input is an iterable container.
    - [Not Asserted] Assume that input is convertible to strings.
    """
    hashlist = [str(arg) for arg in args]
    return "".join(hashlist)


def UniqueList(elements):
    """
    *** Pre-conditions:
    - [Not Asserted] Assume that input can be soft copied to a set by `set(elements)`.
    - [Not Asserted] Assume that input can be soft copied to a list by `list(elements)`.
    """
    return len(set(elements)) == len(list(elements))


def UniqueStrs(elements):
    """
    *** Pre-conditions:
    - [Not Asserted] Assume that input can be soft copied to a list by `list(elements)`.
    - [Not Asserted] Assume that input has a __str__ operator.
    """
    return UniqueList([str(element) for element in list(elements)])


def AbsParity(permutation):
    """
    Given a permutation of arbitrary comparable objects, find its absolute parity (-1/0/+1).
    "Absolute" parity means parity with respect to the sorted sequence of the same elements.

    :param permutation: A permutation of comparable objects
    :return: Absolute parity of the permutation (-1/0/+1)

    *** Pre-conditions:
    - [Not Asserted] Assume that input can be soft copied to a list by `list(permutation)`.
    - [Not asserted] Assume that input has comparable elements so the sorting can work.

    *** Post-conditions:
    - None

    *** Notes:
    - First, check if every element is unique. If there are any repeated elements, return 0.
    - Given a sequence of unique elements, count the number of transpositions (i.e., swaps of arbitrary pairs) that the
        sequence is away from the canonical (i.e., sorted sequence). If it is even/odd, return +1/-1, respectively.
    - Applies swap-sort to select pairs to be swapped and counts the number of such swaps.

    *** TODO:
    - [Low priority] Replace swap-sort with a faster sorting algorithm that works based on swapping elements.

    """

    # Make a copy so we don't mutate the original:
    permutation = list(permutation)

    if len(permutation) == 0:
        return 0  # Convention: Empty list has zero parity.
    if len(permutation) == 1:
        return 1  # Convention: Singleton list has +1 parity.
    # Note: The latter is not necessary. The following algorithm will return +1 anyway for singleton sets.

    if len(set(permutation)) != len(permutation): return 0
    # After this point, it is safe to assume the list has unique elements.

    swap_count = 0
    while permutation:  # While the list is nonempty:
        min_element, min_index = min((min_element, min_index) for (min_index, min_element) in enumerate(permutation))
        if min_index != 0:  # There is a need for swapping:
            permutation[0], permutation[min_index] = permutation[min_index], permutation[0]  # No need for temp_var!
            swap_count += 1
        permutation = permutation[1:]

    if (swap_count % 2) == 0:
        return +1
    else:
        return -1


def RelParity(perm1, perm2):
    """
    Given two permutations of arbitrary objects, check if their relative parity is the same.

    :param perm1, perm2: Two permutations of the same objects
    :return: Relative parity of the permutation (-1/0/+1)

    *** Pre-conditions:
    - [Not Asserted] Assume that inputs can be soft copied to a list by `list(perm1), list(perm2)`.
    - [Asserted] Assume that inputs are sequences with copies from the same set of objects.

    *** Post-conditions:
    - None

    *** Notes:
    - First, check if every element is unique. If there are any repeated elements, return 0.
    - The permutations have the same parity if one can be converted to the other by an even number of transpositions
        (i.e., swaps of arbitrary pairs).
    """

    # Make a copy so we don't mutate the original:
    perm1, perm2 = list(perm1), list(perm2)

    assert (set(perm1) == set(perm2))
    num_elements = len(set(perm1))

    if len(perm1) != num_elements: return 0
    if len(perm2) != num_elements: return 0
    # After this point, it is safe to assume each list has unique elements.

    if num_elements == 0:
        return 0  # Convention: Empty list has zero parity.
    if num_elements == 1:
        return 1  # Convention: Singleton list has +1 parity.
    # Note: None of the above two checks are necessary. The following algorithm will return a consistent answer.

    # The integer locations of perm2 elements in perm1:
    permutation = [perm1.index(perm2[i]) for i in range(num_elements)]

    return AbsParity(permutation)


class PermList:
    """
    A class to represent "permutable lists" as equivalence classes of ordered lists modulo even permutation.

    - Two objects are equal (comparison overloaded) if they are an even number of permutations away.
    - Two objects are opposite (negation overloaded) if they are an odd number of permutations away.
    """

    @property
    def elements(self):
        return self._elements

    @property
    def parity(self):
        return AbsParity(self.elements)

    # Note: This will fail an assert if the vertices are not from a totally ordered set, i.e., not comparable.

    @elements.setter
    def elements(self, elements):
        """
        *** Pre-conditions:
        - [Not Asserted] Assume that input can be soft copied to a list by `list(elements)`.
        - [Not asserted] Assume that input has comparable elements so the sorting can work.
        - [Not Asserted] Assume that input is convertible to strings.
        """
        assert (UniqueStrs(elements))  # Cannot have repeated elements, or vertices with the same string form.
        self._elements = list(elements)
        self._hashkey = CreateHashKey(SignChar(self.parity), sorted(self.elements))

    def __init__(self, other=[]):
        if isinstance(other, PermList):
            self.elements = other.elements
        else:
            assert (isinstance(other, set) or isinstance(other, list) or isinstance(other, tuple))
            self.elements = other

    def __contains__(self, element):
        return element in self.elements

    def __getitem__(self, item):
        return self.elements[item]

    def __iter__(self):
        return iter(self.elements)

    def __next__(self):
        return next(self.elements)

    @property
    def size(self):
        return len(self.elements)

    def __len__(self):
        return self.size

    @property
    def is_empty(self):
        return len(self) == 0

    @property
    def is_singleton(self):
        return len(self) == 1

    def intersection(self, other):
        if isinstance(other, PermList):
            int_elements = [element for element in self if element in other]
            return PermList(int_elements)
        else:
            warnings.warn("Intersecting PermList instance with a different type.", RuntimeWarning)
            return self.intersection(PermList(other))

    # Note: Preserves the order of the 1st object (over the intersection elements).

    def __mul__(self, other):
        return self.intersection(other)

    def difference(self, other):
        if isinstance(other, PermList):
            sub_elements = [element for element in self if element not in other]
            return PermList(sub_elements)
        else:
            warnings.warn("Differencing PermList instance with a different type.", RuntimeWarning)
            return self.difference(PermList(other))

    # Note: Preserves the order of the 1st object (over the difference elements).

    def __sub__(self, other):
        return self.difference(other)

    def union(self, other):
        if isinstance(other, PermList):
            fst_elements = [element for element in self]  # Has to be a list for the '+' below to work.
            sub_elements = [element for element in other if element not in self]
            return PermList(fst_elements + sub_elements)
        else:
            warnings.warn("Differencing PermList instance with a different type.", RuntimeWarning)
            return self.difference(PermList(other))

    # Note: Preserves the order of the 1st object (over all elements) and 2nd object (over difference elements).

    def __add__(self, other):
        return self.union(other)

    def issubset(self, other):
        if isinstance(other, PermList):
            for element in self:
                if element not in other: return False
            return True
        else:
            warnings.warn("Sub-set check of PermList instance with a different type.", RuntimeWarning)
            return self.issubset(PermList(other))

    def issuperset(self, other):
        if isinstance(other, PermList):
            return other.issubset(self)
        else:
            warnings.warn("Super-set check of PermList instance with a different type.", RuntimeWarning)
            return self.issuperset(PermList(other))

    def __hash__(self):
        return hash(str(type(self)) + self._hashkey)

    def __eq__(self, other):
        if isinstance(other, PermList):
            return self._hashkey == other._hashkey
        else:
            warnings.warn("Comparing PermList instance with a different type.", RuntimeWarning)
            return self == PermList(other)

    # Note: Two permutable lists are deemed "equal" if they are an even number of permutations away.

    def __neg__(self):
        if self.is_empty or self.is_singleton:
            return self  # Does not make a copy (of vertices).
        else:
            neg_elements = list(self.elements)  # Makes a shallow copy.
            neg_elements[0], neg_elements[1] = neg_elements[1], neg_elements[0]
            return PermList(neg_elements)

    # Note: Two permutable lists are deemed "opposite" if they are an odd number of permutations away.

    def __ne__(self, other):
        return not (self == other)

    def combinations(self, num_elements):
        return set(PermList(com_elements) for com_elements in combinations(self.elements, num_elements))

    def __str__(self):
        return self._hashkey

    def __repr__(self):
        return "PermList of elements " + str(self)

    def print(self, long=True):
        print(self.__repr__()) if long else print(self.__str__())


class Simplex(ABC):
    """
    The base class to represent abstract (i.e., not embedded) simplices.

    *** Notes:
    - This is an abstract class with placeholders. Use for inheritance (No direct instantiation).
    """

    @property
    @abstractmethod
    def vertices(self):
        raise NotImplementedError

    @vertices.setter
    @abstractmethod
    def vertices(self, vertices):
        raise NotImplementedError

    def __len__(self):
        return len(self.vertices)

    def __contains__(self, vertex):
        return vertex in self.vertices

    @property
    def dim(self):
        return len(self) - 1 if len(self) > 0 else None

    @property
    def is_empty(self):
        return len(self) == 0

    @property
    def is_point(self):
        return self.dim == 0

    @property
    def is_line(self):
        return self.dim == 1

    @property
    def is_surface(self):
        return self.dim == 2

    @property
    def is_volume(self):
        return self.dim == 3

    def intersection(self, other):
        if isinstance(other, Simplex):
            if type(self) != type(other):
                warnings.warn("Intersecting Simplex instances of different types.", RuntimeWarning)
            return (type(self))(self.vertices.intersection(other.vertices))
        else:
            warnings.warn("Intersecting Simplex instance with a different type.", RuntimeWarning)
            return self.intersection((type(self))(other))

    def __mul__(self, other):
        return self.intersection(other)

    def difference(self, other):
        if isinstance(other, Simplex):
            if type(self) != type(other):
                warnings.warn("Differencing Simplex instances of different types.", RuntimeWarning)
            return (type(self))(self.vertices.difference(other.vertices))
        else:
            warnings.warn("Intersecting Simplex instance with a different type.", RuntimeWarning)
            return self.difference((type(self))(other))

    def __sub__(self, other):
        return self.difference(other)

    def union(self, other):
        if isinstance(other, Simplex):
            if type(self) != type(other):
                warnings.warn("Unifying Simplex instances of different types.", RuntimeWarning)
            return (type(self))(self.vertices.union(other.vertices))
        else:
            warnings.warn("Unifying Simplex instance with a different type.", RuntimeWarning)
            return self.union((type(self))(other))

    def __add__(self, other):
        return self.union(other)

    def is_face(self, other):
        if isinstance(other, Simplex):
            return self.vertices.issubset(other.vertices)
        else:
            assert (isinstance(other, set) or isinstance(other, list) or isinstance(other, tuple))
            return self.is_face((type(self))(other))

    # Note: As a convention, an empty simplex is assumed to be a face to every other simplex!

    def is_coface(self, other):
        if isinstance(other, Simplex):
            return other.is_face(self)
        else:
            assert (isinstance(other, set) or isinstance(other, list) or isinstance(other, tuple))
            return self.is_coface((type(self))(other))

    def is_facet(self, other):
        if isinstance(other, Simplex):
            if self.is_empty or other.is_point:
                return self.is_empty and other.is_point
            else:  # if self.dim >= 0 and (other.dim - 1) >= 0:
                return self.dim == (other.dim - 1) and self.is_face(other)
        else:
            assert (isinstance(other, set) or isinstance(other, list) or isinstance(other, tuple))
            return self.is_facet((type(self))(other))

    # Note: A facet is a face that has exactly one fewer vertex than the simplex vertices.

    def is_cofacet(self, other):
        if isinstance(other, Simplex):
            return other.is_facet(self)
        else:
            assert (isinstance(other, set) or isinstance(other, list) or isinstance(other, tuple))
            return self.is_cofacet((type(self))(other))

    # Note: A co-facet is a co-face that has exactly one more vertex than the simplex vertices.

    def is_incident(self, other):
        return self.is_face(other) or self.is_coface(other)

    # Note: Two simplices are incident if one of them is the face/co-face of the other.

    def is_comparable(self, other):
        return self.is_incident(other)

    # Note: Two simplices are comparable (in partial order setting) if they are incident.

    def is_attached(self, other):
        return self.is_facet(other) or self.is_cofacet(other)

    # Note: Two simplices are attched if one of them is the facet/co-facet of the other.

    def is_adjacent(self, other):
        return self.is_empty or other.is_empty or not (self * other).is_empty

    # Note: Two simplices are adjacent if they share at least one face (of any dimension).

    def __hash__(self):
        return hash(str(type(self)) + str(self.vertices))

    def __eq__(self, other):
        if isinstance(other, Simplex):
            if type(self) != type(other):
                warnings.warn("Comparing Simplex instances of different types.", RuntimeWarning)
            return self.vertices == other.vertices
        else:
            warnings.warn("Comparing Simplex instance with a different type.", RuntimeWarning)
            return self == (type(self))(other)

    @abstractmethod
    def __neg__(self):
        raise NotImplementedError

    def __ne__(self, other):
        return not (self == other)

    def __le__(self, other):
        if isinstance(other, Simplex):
            return (self - other).is_empty
        else:
            warnings.warn("Checking inequality of Simplex object with a different type.", RuntimeWarning)
            return self <= (type(self))(other)

    # Note: Since two simplices are not always comparable, __le__ is NOT the negation of __gt__.

    def __ge__(self, other):
        return other <= self

    # Note: Since two simplices are not always comparable, __ge__ is NOT the negation of __lt__.

    def __lt__(self, other):
        return self <= other and self != other

    # Note: Since two simplices are not always comparable, __lt__ is NOT the negation of __ge__.

    def __gt__(self, other):
        return other < self

    # Note: Since two simplices are not always comparable, __gt__ is NOT the negation of __le__.

    def d_faces(self, face_dim):
        assert (isinstance(face_dim, int) and face_dim >= 0)
        if not isinstance(self.dim, int) or face_dim > self.dim:
            return []
        else:
            comb = combinations(self.vertices, face_dim + 1)
            return [(type(self))(face_vertices) for face_vertices in comb]

    # Note: This function assumes `combinations` is defined for the vertices (e.g., `set` or `PermList`)

    def facets(self):
        if self.is_empty:
            return []
        elif self.is_point:
            return [(type(self))()]  # Empty simplex is a facet of 0-simplices.
        else:  # if (self.dim - 1) >= 0:
            return self.d_faces(self.dim - 1)

    def faces(self):
        faces = [self]  # The simplex itself is always included.
        if not self.is_empty:
            for facet in self.facets():
                faces.extend(facet.faces())  # Recursive call
        return faces

    @abstractmethod
    def incidence_coeff(self, other):
        raise NotImplementedError

    @abstractmethod
    def attaching_coeff(self, other):
        raise NotImplementedError

    @abstractmethod
    def adjacency_coeff(self, other):
        raise NotImplementedError

    def __str__(self):
        return str(self.vertices)

    def __repr__(self):
        return str(self.dim) + "-simplex of vertices " + str(self)

    def print(self, long=True):
        print(self.__repr__()) if long else print(self.__str__())


class UnorientedSimplex(Simplex):
    """
    A class to represent abstract (i.e., not embedded) unoriented simplices.

    - Every unoriented simplex is an unordered set of vertices (of any type).
    - Two simplices are equal if their set of vertices is the same, using Python's equality of sets.
    """

    @property
    def vertices(self):
        return self._vertices  # A Python `set` instance

    @vertices.setter
    def vertices(self, vertices):
        if isinstance(vertices, PermList):
            self._vertices = set(vertices.elements)  # Makes a shallow copy.
        else:
            assert (isinstance(vertices, set) or isinstance(vertices, list) or isinstance(vertices, tuple))
            self._vertices = set(vertices)  # Makes a shallow copy.

    def __init__(self, other=set()):
        if isinstance(other, Simplex):
            self.vertices = other.vertices
        elif isinstance(other, PermList):
            self.vertices = other
        else:
            assert (isinstance(other, set) or isinstance(other, list) or isinstance(other, tuple))
            self.vertices = other

    def incidence_coeff(self, other):
        return 1 if self.is_incident(other) else 0

    # Note: incidence coefficient is nonzero (i.e., 1) only for incident (i.e., face/co-face) simplices.

    def attaching_coeff(self, other):
        return 1 if self.is_attached(other) else 0

    # Note: attaching coefficient is nonzero (i.e., 1) only for attached (i.e., facet/co-facet) simplices.

    def adjacency_coeff(self, other):
        return 1 if self.is_adjacent(other) else 0

    # Note: adjacency coefficient is nonzero (i.e., 1) only for adjacent (i.e., intersecting) simplices.

    def __neg__(self):
        return UnorientedSimplex(self.vertices)

    def __repr__(self):
        return "Unoriented " + super().__repr__()


class OrientedSimplex(Simplex):
    """
    A class to represent abstract (i.e., not embedded) oriented simplices.

    - Every oriented simplex is an ordered list of vertices (of any type) modulo even permutations.
    - The orientation of the simplex is implicit in the order of the list modulo even permutations.
    - Two simplices are equal (comparison overloaded) if they are an even number of permutations away.
    - Two simplices are opposite (negation overloaded) if they are an odd number of permutations away.
    """

    @property
    def vertices(self):
        return self._vertices  # A `PermList` instance

    @vertices.setter
    def vertices(self, vertices):
        if isinstance(vertices, PermList):
            self._vertices = PermList(vertices)  # Makes a shallow copy.
        else:
            assert (isinstance(vertices, set) or isinstance(vertices, list) or isinstance(vertices, tuple))
            self._vertices = PermList(vertices)

    @property
    def orientation(self):
        return self.vertices.parity  # +1 (even parity) or -1 (odd parity)

    def __init__(self, other=PermList()):
        if isinstance(other, Simplex):
            self.vertices = other.vertices
        elif isinstance(other, PermList):
            self.vertices = other
        else:
            assert (isinstance(other, set) or isinstance(other, list) or isinstance(other, tuple))
            self.vertices = other

    def compatibility(self, other):
        if self.is_face(other):
            simplex1 = self + (other - self)
            simplex2 = other
        elif self.is_coface(other):
            simplex1 = self
            simplex2 = other + (self - other)
        else:  # This is the general case. Previous if conditions are for optimization.
            simplex1 = self + (other - self)
            simplex2 = other + (self - other)
        return RelParity(simplex1.vertices, simplex2.vertices)

    # Note: Compatibility (of orientation) is defined for every pair of simplices, even disjoint ones (no meaning).

    def incidence_coeff(self, other):
        return self.compatibility(other) if self.is_incident(other) else 0

    # Note: incidence coefficient is nonzero (i.e., -/+1) only for incident (i.e., face/co-face) simplices.

    def attaching_coeff(self, other):
        return self.compatibility(other) if self.is_attached(other) else 0

    # Note: attaching coefficient is nonzero (i.e., -/+1) only for attached (i.e., facet/co-facet) simplices.

    def adjacency_coeff(self, other):
        return self.compatibility(other) if self.is_adjacent(other) else 0

    # Note: adjacency coefficient is nonzero (i.e., -/+1) only for adjacent (i.e., intersecting) simplices.

    def __neg__(self):
        return OrientedSimplex(-self.vertices)

    def __repr__(self):
        return "Oriented " + super().__repr__()


class SimplicialComplex:
    """
    A class to represent abstract (i.e., not embedded) (un)oriented simplicial complexes.

    *** Notes:
    - The simplices of various dimensions are all stored in a single dictionary.
    - The dictionary maps a given d in [None, 0, 1, ...] to a dictionary of d-simplices.
    - As a convention, the empty simplex (corresponding to `None`) is always included.
    - The `add` and `remove` functions are the only mechanisms to add and remove simplices.
    - When a simplex is added/removed, all of its faces/co-faces must be added/removed too.
    """

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, name):
        assert (isinstance(name, str))
        self._name = name

    @property
    def bottom(self):
        (bottom,) = self._simplex_dict[None].keys()  # OrientedSimplex() or UnorientedSimplex()
        return bottom

    # Note: This fails if the set `self._simplex_dict[None]` is not singleton, which has to be!

    @property
    def is_oriented(self):
        return type(self.bottom) == OrientedSimplex

    @property
    def dim(self):
        dims = [dim for dim in self._simplex_dict.keys() if isinstance(dim, int)]
        if dims:  # If it is nonempty:
            return max(dims)
        else:
            return None

    @property
    def is_empty(self):
        return len(self._simplex_dict.keys()) == 1

    # Note: The empty simplicial complex is defined as one that includes only the empty simplex.

    def reset(self, oriented=False):
        if oriented:
            self._simplex_dict = {None: {OrientedSimplex(): "Null"}}
        else:
            self._simplex_dict = {None: {UnorientedSimplex(): "Null"}}

    # Note: As a bare minimum, the dictionary contains a map from `None` to the empty simplex.

    def __contains__(self, simplex):
        if isinstance(simplex, type(self.bottom)):
            if simplex.dim in self._simplex_dict.keys():
                if simplex in self._simplex_dict[simplex.dim].keys():
                    return True
            return False
        else:
            return (type(self.bottom))(simplex) in self

    def d_simplices(self, simplex_dim):
        if simplex_dim in self._simplex_dict.keys():
            return list(self._simplex_dict[simplex_dim].keys())  # Makes a shallow copy.
        else:
            return []

    # Note: This function uses a meaningless yet deterministic ordering (to use in incidence matrix).
    # Note: The ordering of d-simplices is lexicographic in terms of their unique string representations.

    def d_faces(self, simplex, face_dim):
        return [face for face in self.d_simplices(face_dim) if face.is_face(simplex)]

    def d_cofaces(self, simplex, coface_dim):
        return [coface for coface in self.d_simplices(coface_dim) if coface.is_coface(simplex)]

    def facets(self, simplex):
        if simplex.is_empty:
            return []
        elif simplex.is_point:
            return [self.bottom]  # Empty simplex is a facet of 0-simplices.
        else:  # if (simplex.dim - 1) >= 0:
            return self.d_faces(simplex, simplex.dim - 1)

    def cofacets(self, simplex):
        return self.d_cofaces(simplex, simplex.dim + 1)

    # Note: The simplex itself may not in the simplicial complex, but its faces/co-faces may be.

    def d_size(self, simplex_dim):
        if simplex_dim in self._simplex_dict.keys():
            return len(self._simplex_dict[simplex_dim].keys())
        else:
            return 0

    def simplices(self):
        simplex_list = []
        for simplex_dim in self._simplex_dict.keys():
            simplex_list.extend(self.d_simplices(simplex_dim))
        return simplex_list

    def faces(self, simplex):
        return [face for face in self.simplices() if face.is_face(simplex)]

    def cofaces(self, simplex):
        return [coface for coface in self.simplices() if coface.is_coface(simplex)]

    @property
    def size(self):
        return sum(self.d_size(simplex_dim) for simplex_dim in self._simplex_dict.keys())

    def __len__(self):
        return self.size

    def is_maximal(self, simplex):
        if isinstance(simplex, type(self.bottom)):
            if simplex.is_empty:
                return self.is_empty  # The empty simplex is maximal in empty complex only.
            elif (simplex.dim + 1) in self._simplex_dict.keys():
                for big_simplex in self.d_simplices(simplex.dim + 1):
                    if simplex.is_face(big_simplex): return False
                return True
            else:
                return True
        else:
            return self.is_maximal((type(self.bottom))(simplex))

    def max_simplices(self):
        return [simplex for simplex in self.simplices() if self.is_maximal(simplex)]

    def _add_unsafe(self, *simplices):
        for simplex in simplices:
            simplex_dim = simplex.dim
            if simplex_dim not in self._simplex_dict.keys():
                self._simplex_dict[simplex_dim] = dict()
            if self.is_oriented:
                if -simplex not in self._simplex_dict[simplex_dim].keys():
                    self._simplex_dict[simplex_dim][simplex] = "PLACEHOLDER"
            else:
                self._simplex_dict[simplex_dim][simplex] = "PLACEHOLDER"

    # Note: This function does NOT add all faces, resulting in an invalid simplicial complex.

    def _remove_unsafe(self, *simplices):
        for simplex in simplices:
            simplex_dim = simplex.dim
            if simplex_dim in self._simplex_dict.keys():
                self._simplex_dict[simplex_dim].pop(simplex, None)
                if self.is_oriented and -simplex in self._simplex_dict[simplex_dim]:
                    self._simplex_dict[simplex_dim].opo(-simplex, None)
                if not self._simplex_dict[simplex_dim].keys():
                    self._simplex_dict.pop(simplex_dim, None)

    # Note: This function does NOT remove all co-faces, resulting in an invalid simplicial complex.

    def add(self, *max_simplices):
        for max_simplex in max_simplices:
            if not isinstance(max_simplex, type(self.bottom)):
                max_simplex = (type(self.bottom))(max_simplex)
            if not max_simplex.is_empty:
                for face_dim in range(0, max_simplex.dim + 1):
                    self._add_unsafe(*max_simplex.d_faces(face_dim))

    # Note: when adding an oriented simplex, if -simplex is in the complex, nothing will be added.

    def remove(self, *min_simplices):
        for min_simplex in min_simplices:
            if not isinstance(min_simplex, type(self.bottom)):
                min_simplex = (type(self.bottom))(min_simplex)
            if not min_simplex.is_empty:
                for coface_dim in range(min_simplex.dim, self.dim + 1):
                    self._remove_unsafe(*self.d_cofaces(min_simplex, coface_dim))

    # Note: when removing an oriented simplex, if -simplex is in the complex, -simplex is removed.

    def add_batch(self, *args):
        for arg in args:
            for sub_arg in arg:
                self.add(sub_arg)

    # Note: This is for the case when a tuple of sets/lists of simplices is given as input.

    def remove_batch(self, *args):
        for arg in args:
            for sub_arg in arg:
                self.remove(sub_arg)

    # Note: This is for the case when a tuple of sets/lists of simplices is given as input.

    def __init__(self, name="UnnamedComplex", oriented=False, *max_simplices):
        self.reset(oriented)
        self.add(max_simplices)
        self.name = name

    def incidence_matrix(self, simplex_dim1, simplex_dim2, fast=True):

        dtype = np.int8  # if self.is_oriented else np.bool_

        if simplex_dim1 is None and simplex_dim2 is None:
            return sparse.identity(1, dtype, "csr")
        elif simplex_dim1 is None:
            assert (isinstance(simplex_dim2, int) and 0 <= simplex_dim2 <= self.dim)
            return sparse.csr_matrix(np.zeros((1, self.d_size(simplex_dim2)), dtype))
        elif simplex_dim2 is None:
            assert (isinstance(simplex_dim1, int) and 0 <= simplex_dim1 <= self.dim)
            return sparse.csr_matrix(np.zeros((self.d_size(simplex_dim1), 1), dtype))
        else:
            assert (isinstance(simplex_dim1, int) and 0 <= simplex_dim1 <= self.dim)
            assert (isinstance(simplex_dim2, int) and 0 <= simplex_dim2 <= self.dim)

        if fast:  # Faster but dirtier implementation:
            if simplex_dim1 == simplex_dim2:
                return sparse.identity(self.d_size(simplex_dim1), dtype, "csr")
            elif simplex_dim1 < simplex_dim2:
                size1, size2 = self.d_size(simplex_dim1), self.d_size(simplex_dim2)
                simplices1 = self.d_simplices(simplex_dim1)
                simplices2 = self.d_simplices(simplex_dim2)
                row_indices, col_indices, matrix_data = [], [], []
                if self.is_oriented:
                    for index2 in range(size2):
                        simplex2 = simplices2[index2]
                        simplices2_faces = simplex2.d_faces(simplex_dim1)
                        for face in simplices2_faces:
                            if face in simplices1:
                                index1 = simplices1.index(face)
                                row_indices.append(index1)
                                col_indices.append(index2)
                                matrix_data.append(simplex2.compatibility(face))
                            elif -face in simplices1:
                                index1 = simplices1.index(-face)
                                row_indices.append(index1)
                                col_indices.append(index2)
                                matrix_data.append(simplex2.compatibility(face))
                else:
                    for index2 in range(size2):
                        simplices2_faces = simplices2[index2].d_faces(simplex_dim1)
                        for face in simplices2_faces:
                            if face in simplices1:
                                index1 = simplices1.index(face)
                                row_indices.append(index1)
                                col_indices.append(index2)
                                matrix_data.append(1)
                return sparse.csr_matrix((matrix_data, (row_indices, col_indices)), shape=(size1, size2), dtype=dtype)
            else:  # if simplex_dim1 > simplex_dim2:
                return self.incidence_matrix(simplex_dim2, simplex_dim1, True).transpose()

        else:  # Cleaner but slower implementation:

            matrix_shape = size1, size2 = self.d_size(simplex_dim1), self.d_size(simplex_dim2)
            simplices1 = self.d_simplices(simplex_dim1)
            simplices2 = self.d_simplices(simplex_dim2)

            row_indices, col_indices, matrix_data = [], [], []

            for index1, index2 in np.ndindex(matrix_shape):
                incidence_coeff = simplices1[index1].incidence_coeff(simplices2[index2])
                if incidence_coeff != 0:
                    row_indices.append(index1)
                    col_indices.append(index2)
                    matrix_data.append(incidence_coeff)

            return sparse.csr_matrix((matrix_data, (row_indices, col_indices)), shape=matrix_shape, dtype=dtype)

    def __str__(self):
        str_repr = self.name + " = {"
        for simplex in self.simplices():
            str_repr += str(simplex) + ", "
        str_repr = str_repr[:-2] + '}'
        return str_repr

    def __repr__(self):
        str_repr = "Oriented " if self.is_oriented else "Unoriented "
        str_repr += str(self.dim) + "-complex '" + self.name + "' consisting of:" + "\n"
        for simplex_dim in self._simplex_dict.keys():
            str_repr += str(simplex_dim) + "-simplices: "
            for simplex in self.d_simplices(simplex_dim):
                str_repr += str(simplex) + ", "
            str_repr = str_repr[:-2] + "\n"
        return str_repr[:-1]

    def print(self, long=True):
        print(self.__repr__()) if long else print(self.__str__())

    def draw(self, vertical=False, spacing=0.75, **kwargs):
        show_bottom = kwargs["show_bottom"] if "show_bottom" in kwargs.keys() else True
        drawing_graph = nx.DiGraph()
        drawing_graph.graph["node"] = {"shape": "circle"}
        drawing_graph.graph["edge"] = {"arrowsize": "0.6", "splines": "curved"}
        drawing_graph.graph["graph"] = {"scale": "3"}

        # Draw the bottom node (i.e., single leaf) for empty simplex:
        if show_bottom:
            node_position = '-' + str(spacing) + ", 0!" if vertical else "0, -" + str(spacing) + '!'
            if self.is_oriented:
                drawing_graph.add_node(OrientedSimplex())
                node_color = "green" if self.is_maximal(OrientedSimplex()) else "cyan"
                drawing_graph.nodes[OrientedSimplex()]["pos"] = node_position
                drawing_graph.nodes[OrientedSimplex()]["label"] = OrientedSimplex().__str__()
                drawing_graph.nodes[OrientedSimplex()]["style"] = "filled"
                drawing_graph.nodes[OrientedSimplex()]["fillcolor"] = node_color
            else:
                drawing_graph.add_node(UnorientedSimplex())
                node_position = "-1, 0!" if vertical else "0, -1!"
                node_color = "green" if self.is_maximal(UnorientedSimplex()) else "cyan"
                drawing_graph.nodes[UnorientedSimplex()]["pos"] = node_position
                drawing_graph.nodes[UnorientedSimplex()]["label"] = UnorientedSimplex().__str__()
                drawing_graph.nodes[UnorientedSimplex()]["style"] = "filled"
                drawing_graph.nodes[UnorientedSimplex()]["fillcolor"] = node_color

        # Draw the remaining nodes, for all the d-simplices (d >= 0):
        if self.dim is not None:
            for simplex_dim in range(self.dim + 1):
                simplices = self.d_simplices(simplex_dim)
                shift = len(simplices) / 2
                for simplex_index in range(len(simplices)):
                    simplex = simplices[simplex_index]
                    drawing_graph.add_node(simplex)
                    if vertical:
                        node_position = str(simplex_dim) + ", " + str(spacing * (simplex_index - shift)) + "!"
                    else:
                        node_position = str(spacing * (simplex_index - shift)) + ", " + str(simplex_dim) + "!"
                    node_color = "green" if self.is_maximal(simplex) else "cyan"
                    drawing_graph.nodes[simplex]["pos"] = node_position
                    drawing_graph.nodes[simplex]["label"] = simplex.__str__()
                    drawing_graph.nodes[simplex]["style"] = "filled"
                    drawing_graph.nodes[simplex]["fillcolor"] = node_color

        # Draw the arrows for the bottom node (connected to all 0-simplices):
        if show_bottom:
            if self.is_oriented:
                for vertex in self.d_simplices(0):
                    drawing_graph.add_edge(OrientedSimplex(), vertex)
                    drawing_graph.edges[OrientedSimplex(), vertex]["label"] = ""
                    drawing_graph.edges[OrientedSimplex(), vertex]["color"] = "black"
            else:
                for vertex in self.d_simplices(0):
                    drawing_graph.add_edge(UnorientedSimplex(), vertex)
                    drawing_graph.edges[UnorientedSimplex(), vertex]["label"] = ""
                    drawing_graph.edges[UnorientedSimplex(), vertex]["color"] = "black"

        # Draw the arrows for all the d-simplices to (d+1)-simplices (d >= 0):
        if self.dim is not None:
            for simplex_dim in range(self.dim):
                simplices1 = self.d_simplices(simplex_dim)
                simplices2 = self.d_simplices(simplex_dim + 1)
                for simplex1 in simplices1:
                    for simplex2 in simplices2:
                        edge_weight = simplex1.incidence_coeff(simplex2)
                        if edge_weight != 0:
                            drawing_graph.add_edge(simplex1, simplex2)
                            if self.is_oriented:
                                if edge_weight == +1:
                                    drawing_graph.edges[simplex1, simplex2]["label"] = '+'
                                    drawing_graph.edges[simplex1, simplex2]["color"] = "blue"
                                if edge_weight == -1:
                                    drawing_graph.edges[simplex1, simplex2]["label"] = '–'
                                    drawing_graph.edges[simplex1, simplex2]["color"] = "red"
                            else:
                                drawing_graph.edges[simplex1, simplex2]["label"] = ""
                                drawing_graph.edges[simplex1, simplex2]["color"] = "black"

        return drawing_graph

    def load(self, vertical=False, file_name=None):

        assert (isinstance(file_name, str))
        if file_name[-5:] == ".json":
            with open(file_name) as in_file:
                data = json.load(in_file)
            if "name" in data.keys():
                self.name = name
                data.pop("name", None)
            if "is_oriented" in data.keys():
                self.reset(data["is_oriented"])
                data.pop("is_oriented", None)
            else:
                self.reset(self.is_oriented)

            max_simplices = [vertices for (name, vertices) in data.items()]
            self.add(*max_simplices)

        else:
            warnings.warn("File format not recognized. Loading nothing...")

    def save(self, vertical=False, file_name=None):

        assert (isinstance(file_name, str))
        if file_name[-5:] == ".json":
            max_simplices = self.max_simplices()
            data = {}
            data["name"] = self.name
            data["is_oriented"] = self.is_oriented
            for index in range(len(max_simplices)):
                data[index] = list(max_simplices[index].vertices)
            with open(file_name, 'w') as out_file:
                json.dump(data, out_file, indent=4)

        elif file_name[-4:] == ".dot":
            graph = self.draw(vertical)
            if file_name is not None:
                write_dot(graph, file_name)

        else:
            warnings.warn("File format not recognized. Loading nothing...")

    def plot(self, vertical=False, file_name=None):
        agraph = to_agraph(self.draw(vertical))
        agraph.layout(prog="neato")
        if file_name is not None:
            agraph.draw(file_name)
