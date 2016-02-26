class Tree(object):
    
    class Node(object):
        def __init__(self, index, splitter):
            self.index = index
            self.splitter = splitter
            self.left = None
            self.right = None

    def __init__(self):
        self.root = self.Node(None, None)
        self.depth = 0

    def __str__(self):
        if self.root is not None:
            self._print(self.root)

        return ''

    def _print(self, node):
        self.depth += 1
        if node is not None:
            print('\t' * (self.depth - 1), node.index.size, node.splitter)
            self._print(node.left)
            self._print(node.right)
        
        self.depth -= 1

    def insert(self, data):
        if self.root is None:
            self.root = self.Node(data)

        else:
            self._insert(self.root, data)

    def _insert(self, node, data):
        if node.data < data:
            if node.right is None:
                node.right = self.Node(data)
            else:
                self._insert(node.right, data)
                
        else:
            if node.left is None:
                node.left = self.Node(data)
            else:
                self._insert(node.left, data)