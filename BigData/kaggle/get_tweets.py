import tweepy
import json
import sys
import pandas as pd
import MySQLdb


# CONFIGURING API
CONSUMER_KEY = "eI4qKuy2jETIr1YT1VrNwczRP"
CONSUMER_SECRET = "gcQhulnmylPplR517a4lEZRC52myeS7oRrPil1sD0Zf1jeyWyb"

auth = tweepy.AppAuthHandler(CONSUMER_KEY, CONSUMER_SECRET)
api = tweepy.API(auth, wait_on_rate_limit=True, wait_on_rate_limit_notify=True)


# DEFINE
TRAIN_SET_URL = "twitter_train.csv"
TRAIN_TWEETS_TABLE = 'tweets_train'

TEST_SET_URL = "twitter_test.csv"
TEST_TWEETS_TABLE = 'tweets_test'

COUNT = 200
MAX_TWEETS_NUM = 3 * COUNT


# GETTING TWEETS
def get_tweets(users_ids, table_name):
    db = MySQLdb.connect(user="root", passwd="root", db="tweets")
    users_num = len(users_ids)

    for user_i, user_id in enumerate(users_ids):
        params = {
            'user_id': user_id,
            'count': COUNT,
            'include_rts': True,
        }

        tweets = []
        try:
            for status in tweepy.Cursor(api.user_timeline, **params).items(MAX_TWEETS_NUM):
                tweets += [status._json]
        except:
            pass

        stats = {
            'progress': 100.0 * (user_i + 1) / users_num,
            'user_i': user_i + 1,
            'tweets_count': len(tweets),
        }
        print("{progress:.2f}%: {user_i}/{0} | Downloaded {tweets_count} tweets"
              .format(users_num, **stats))

        db.cursor().execute("INSERT INTO {} VALUES(%s, %s);".format(table_name),
                            (user_id, json.dumps(tweets)))
        db.commit()

        del(tweets)

    db.close()


def main():
    # FOR TRAINING SET
    print("Loading train ids...")
    df_users = pd.read_csv(TRAIN_SET_URL)
    users_ids = df_users['twitter_id'].values

    print("Loading tweets for train ids...")
    get_tweets(users_ids, TRAIN_TWEETS_TABLE)

    del(df_users)
    del(users_ids)
    print("Train data loaded", end='\n\n')

    # FOR TESTING SET
    print("Loading test ids...")
    df_users = pd.read_csv(TRAIN_SET_URL)
    users_ids = df_users['twitter_id'].values

    print("Loading tweets for test ids...")
    get_tweets(users_ids, TEST_TWEETS_TABLE)

    del(df_users)
    del(users_ids)
    print("Test data loaded")
