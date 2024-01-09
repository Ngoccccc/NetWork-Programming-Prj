#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "user.h"

UserNode *createUserNode(char *username, char *passwd)
{
  // TODO
  UserNode *newnode = (UserNode *)malloc(sizeof(UserNode));
  strcpy(newnode->username, username);
  strcpy(newnode->password, passwd);
  newnode->room_id = -1;
  newnode->status = OFFLINE;
  newnode->right = NULL;
  newnode->left = NULL;
  return newnode;
}

void traverseUserTree(UserNode *root)
{
  // TODO
  if (root == NULL)
    return;
  traverseUserTree(root->left);
  printf("\n<%s><%s><%d>", root->username, root->password, root->status);
  traverseUserTree(root->right);
}

UserNode *addUser(UserNode *root, char *username, char *passwd)
{
  // TODO
  if (root == NULL)
    root = createUserNode(username, passwd);
  else
  {
    UserNode *node = searchUser(root, username);
    if (node == NULL)
    {
      if (strcmp(root->username, username) < 0)
        root->right = addUser(root->right, username, passwd);
      if (strcmp(root->username, username) > 0)
        root->left = addUser(root->left, username, passwd);
    }
  }
  return root;
}

UserNode *searchUser(UserNode *root, char *username)
{
  // TODO
  if (root == NULL)
    return NULL;
  else
  {
    if (strcmp(root->username, username) < 0)
      root = searchUser(root->right, username);
    else
    {
      if (strcmp(root->username, username) > 0)
        root = searchUser(root->left, username);
    }
  }
  return root;
}

int updateUserStatus(UserNode *root, char *username, UserStatus status)
{
  // TODO
  if (root == NULL)
    return -1;
  else
  {
    UserNode *node = searchUser(root, username);
    if (node != NULL)
    {
      if (node->status == status)
        return 0;
      node->status = status;
    }
  }
  return 1;
}

UserNode *deleteUserNode(UserNode *root, char *username)
{
    if (root == NULL)
        return root;

    // Find the node to be deleted using searchUser
    UserNode *nodeToDelete = searchUser(root, username);

    if (nodeToDelete == NULL) {
        // Node with the given username doesn't exist
        return root;
    }

    // Perform deletion based on whether the node has children
    if (nodeToDelete->left == NULL) {
        // Node has no left child or one child
        UserNode *temp = nodeToDelete->right;
        free(nodeToDelete);
        return temp;
    } else if (nodeToDelete->right == NULL) {
        // Node has no right child
        UserNode *temp = nodeToDelete->left;
        free(nodeToDelete);
        return temp;
    } else {
        // Node with two children, find the in-order successor
        UserNode *temp = nodeToDelete->right;
        while (temp->left != NULL) {
            temp = temp->left;
        }

        // Copy the in-order successor's data to this node
        strcpy(nodeToDelete->username, temp->username);
        strcpy(nodeToDelete->password, temp->password);
        nodeToDelete->room_id = temp->room_id;
        nodeToDelete->status = temp->status;

        // Delete the in-order successor
        nodeToDelete->right = deleteUserNode(nodeToDelete->right, temp->username);

        return root;
    }
}



