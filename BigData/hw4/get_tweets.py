import twitter
import json
import time
import sys
import pandas as pd

from collections import defaultdict


print "Starting program"


CONSUMER_KEY = "eI4qKuy2jETIr1YT1VrNwczRP"
CONSUMER_SECRET = "gcQhulnmylPplR517a4lEZRC52myeS7oRrPil1sD0Zf1jeyWyb"

ACCESS_TOKEN_KEY = "3819130643-iwnyEY9D3YLmYaCGQVy9Qy9fkf8BI7XNqQIdNPF"
ACCESS_TOKEN_SECRET = "ASmwVh4GDBm7BkaxHJoEz7OxO9UVLqthJzvB39YuPyH5p"


api = twitter.Api(consumer_key=CONSUMER_KEY, 
                  consumer_secret=CONSUMER_SECRET, 
                  access_token_key=ACCESS_TOKEN_KEY, 
                  access_token_secret=ACCESS_TOKEN_SECRET)


TWEETS_FILE_URL = 'tweets.json'
TRAINING_SET_URL = "twitter_train.csv"


df_users = pd.read_csv(TRAINING_SET_URL)
users_ids = df_users['twitter_id'].values
del(df_users)


def getTweets(user_id, count=200, include_rts=False,
             trim_user=True, exclude_replies=True):
    
    params = {
        'user_id': user_id,
        'include_rts': include_rts,
        'trim_user': trim_user,
        'exclude_replies': exclude_replies,
    }

    max_id = None
    gotTweets = 0
    timeline = []
    
    while True:
        try:
            tmp = api.GetUserTimeline(max_id=max_id, count=count, **params)

        except twitter.TwitterError as e:
            if isinstance(e.message, list):
                if e.message[0].get("code") == 88: # RATE_LIMIT_EXCEEDED
                    sleep_time = api.GetSleepTime('statuses/user_timeline')
                    for i in xrange(sleep_time, 0, -1):
                        time.sleep(1)
                        sys.stdout.write("%s seconds remaining...                    \r" % i)
                        sys.stdout.flush()
                    continue

            elif isinstance(e.message, basestring):
                if e.message == "Not authorized.":
                    print "\nNot authorized for user with id=%s\n" % user_id
                    tmp = []

            else:
                tmp = []

        if not tmp:
            break

        max_id = tmp[-1].id - 1
        timeline += tmp

        break # for one request for one user

        gotTweets = len(timeline)
        if gotTweets >= 150:
            count = 55;

        if gotTweets >= 200:
            break


    return timeline


tweets = defaultdict(list)

users_num = len(users_ids)

for user_i, user_id in enumerate(users_ids):
    tweets_lst = getTweets(user_id)

    tweets[user_id] += [tweet.AsDict() for tweet in tweets_lst]
    
    stats = {
        'progress': 100.0 * (user_i + 1) / users_num,
        'user_i': user_i + 1,
        'users_count': len(tweets.keys()),
        'tweets_count': len(tweets[user_id]),
    }
    sys.stdout.write(("{progress:.2f}%: {user_i}/{0} | Downloaded {tweets_count} tweets          \r").format(users_num, **stats))
    sys.stdout.flush()

with open(TWEETS_FILE_URL, 'w') as outfile:
    print "\nDumping Tweets to %s ..." % TWEETS_FILE_URL
    json.dump(tweets, outfile)