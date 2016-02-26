from Tree import Tree

import numpy as np
from scipy.spatial import distance_matrix

class DecisionTree(Tree):
    
    def __init__(self, criterion='gini', max_depth=100, min_samples_leaf=1, class_selector='kNN'):
        """
        criterion : string, optional (default="gini")
            The function to measure the quality of a split. Supported criteria are
            "gini" for the Gini impurity and "entropy" for the information gain.
        """
        super().__init__()
        self.criterion = criterion
        self.max_depth = max_depth
        self.min_samples_leaf = min_samples_leaf
        self.class_selector = class_selector
    
    def fit(self, X, y):
        self.X = X
        self.y_orig = y
        self.y_indxs = {n: i for i, n in enumerate(sorted(set(y)))}
        self.y_names = {i: n for n, i in self.y_indxs.items()}
        self.y = np.empty_like(y, dtype=np.int)
        for i in range(y.size):
            self.y[i] = self.y_indxs[y[i]]

        if self.criterion == 'gini':
            self._fit(self.root, np.arange(X.shape[0]))

#         elif self.criterion == 'entropy':
#             self._fit(self.root, np.arange(X.shape[0]), self.entropy)
            
        else:
            return "Error"
        
    def _fit(self, node, index, imp=1):
        self.depth += 1
        node.index = index
        if index.size > self.min_samples_leaf and \
            self.depth < self.max_depth and \
            imp > 1e-5:
            min_info = 10000 #2 * self.y.size
            for i, j in enumerate(self.X[index].T):
                argindx = np.argsort(j)
                ys = self.y[index][argindx]
                
                k = 0
                
                x_l = np.zeros(len(self.y_names))
                x_l[ys[k]] += 1
                s_l = np.sum(x_l)
                g_l = 0

                x_r = np.unique(ys[1:].tolist() + list(self.y_indxs.values()),
                                return_counts=True)[1] - 1
                s_r = np.sum(x_r)
                g_r = 1 - np.sum(x_r ** 2) / (s_r ** 2)
                
                n_l = 1
                n_r = ys.size - n_l
                
                crit = (n_l * g_l + n_r * g_r) / ys.size
                if crit < min_info:
                    min_info = crit
                    min_g_l = g_l
                    min_g_r = g_r
                    splitter = (i, j[argindx[k]])

                for y in ys[1:-1]:
                    k += 1
                    
                    n_l += 1
                    s_l += 1
                    x_l[y] += 1
                    g_l = 1 - (((s_l - 1) ** 2) * (1 - g_l) + 2 * x_l[y] - 1) / (s_l ** 2)
                    
                    n_r -= 1
                    s_r -= 1
                    x_r[y] -= 1
                    g_r = 1 - (((s_r + 1) ** 2) * (1 - g_r) - 2 * x_r[y] - 1) / (s_r ** 2)
                    
                    crit = (n_l * g_l + n_r * g_r) / ys.size
                    if crit < min_info:
                        min_info = crit
                        min_g_l = g_l
                        min_g_r = g_r
                        splitter = (i, j[argindx[k]])

            indL = index[self.X[index, splitter[0]] <= splitter[1]]
            indR = index[self.X[index, splitter[0]] > splitter[1]]
        
            node.splitter = splitter

            node.left = self.Node(None, None)
            self._fit(node.left, indL, min_g_l)

            node.right = self.Node(None, None)
            self._fit(node.right, indR, min_g_r)
        
        self.depth -= 1

    def predict(self, X):
        y = []
        for x in X:
            if self.class_selector == 'kNN':
                y += [self._predict(x, self.root, self.kNN)]
            
            elif self.class_selector == 'max':
                y += [self._predict(x, self.root, self.maxClass)]
            
        return np.array(y).ravel()
    
    def _predict(self, x, node, F):
        if node.splitter is None:
            return self.y_names[F(node.index, x)]
        
        if x[node.splitter[0]] <= node.splitter[1]:
            return self._predict(x, node.left, F)

        else:
            return self._predict(x, node.right, F)
    
    def maxClass(self, index, x):
        ys = np.unique(self.y[index], return_counts=True)
        return ys[0][ys[1].argmax()]
    
    def kNN(self, index, x, neighborsNum = 5):
        """
            index: numpy.ndarray - Nx1, leaf data index
            x: float64, predict data
            neighborsNum: number of neighbors
        """
        x = np.array([x])
        dist = distance_matrix(x, self.X[index])
            
        knn = self.y[index][dist.argsort(axis=1)[:, :neighborsNum]]
        ret = []
        for n in knn:
            tmp = np.unique(n, return_counts=True)
            ret += [tmp[0][tmp[1].argmax()]]
        
        return ret[0]