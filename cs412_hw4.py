import sys
from sys import stdin


# Takes system input and returns a 2D array of the transactions
# Each row is a transaction and contains a list of items
def grab_input():
    
    stop = ['a', 'an', 'are', 'as', 'at', 'by', 'be', 
            'for', 'from', 'has', 'he', 'in', 'is', 'it', 'its', 
            'of', 'on', 'that', 'the', 'to', 'was', 'were', 'will', 'with']
    
    result = []
    
    for line in stdin:
        # Make the entire line lowercase
        line = line.lower().replace('.','')
        
        # Obliterate stop words
        linewords = line.split()
        resultwords = [word for word in linewords if word not in stop]
        
        # DEBUGGING: check the lists
        # print(resultwords)
        
        # Check for commas and 'and's
        item = []
        was_and = False
        last_item = False
        entry = []

        for word in resultwords:

            if word == 'and':
                was_and = True
                continue
            
            if last_item:
                if was_and:
                    item.append(word)
                    entry.append(item)
                    item = []
                    last_item = False
                else:
                    entry.append(item)
                    item = []
                    item.append(word)

            # check if part of an item
            elif ',' in word:
                item.append(word.replace(',',''))
                last_item = False
            # create a new item
            else:
                item.append(word)
                last_item = True
            
            was_and = False
        
        entry.append(item)
        result.append(entry)
        
    return result

def frequent_1_items(customers, minsup):
    
    items = {}
    customer_items = []
    
    # iterate through everything
    for customer in customers:
        #print("Customer has the following transactins:",customer)
        for transaction in customer:
            for item in transaction:
                if item not in items:
                    items[item] = 1
                    customer_items.append(item)
                elif item not in customer_items:
                    items[item] += 1
                    customer_items.append(item)
        customer_items = []
    f_items = {key:value for key, value in items.items() if value >= minsup} 

    return f_items

# 2-sequence generator for GSP algorithm
def gen_seq(fk):
    #print('We need to generate a new list of sequences from',fk)
    
    fn = []
    
    # create all options
    for item in fk:
        itemlist = [item]*len(fk)
        newitems = zip(itemlist,fk)
        newitems = [list(newitem) for newitem in newitems]
        fn += newitems

        
    fj = list(fk) 
    for item_k in fk:
        fj.remove(item_k)    

        for item_j in fj:
            fn.append((item_k, item_j))
    
    #print(fn)
    return fn

# >2-sequence generator for GSP algorithm
def big_gen_seq(fk):
    #print('We need to generate a new list of sequences from',fk)
    
    fn = []
    
    # create all options
    for item in fk:
        #print('We are going to compare everything with',item)
        for item2 in fk:
            #print('Does',item2,'work?')
            if isinstance(item, list) and isinstance(item2, list):
                #print('Lettuce check!')
                #print(item[1:] == item2[:-1])
                if item[1:] == item2[:-1]:
                    #print(item,'and',item2,'make',item + [item2[-1]])
                    fn.append(item + [item2[-1]])
    
    #print(fn)
    return fn    

# GSP
def GSP(customers, f1_dict, minsup):
    
    done = False
    fk_dict = []
    fk = sorted([key for key,vals in f1_dict.items()])
    fn_dict = []
    fn = gen_seq(fk)
    
    while not done:
    
        # run algorithm until there are no patterns left
        # generate n-item sequences, where n > 2
        fn_pruned = []
        
        # count the generated sequences
        for sequence in fn:
            is_frequent = False
            count = 0
            #print('We are counting',sequence)
            if isinstance(sequence, list):
                for customer in customers:
                    #print('We are checking for',sequence,'in',customer)
                    test_sequence = list(sequence)
                    for transaction in customer:
                        try:
                            item = test_sequence[0]
                            #print('We are on item',item)
                        except:
                            item = []
                        
                        if item in transaction:
                            #(item,'is in',transaction)
                            test_sequence.remove(item)
                            #print('Look for the rest of the items in',test_sequence)
                    if not test_sequence:
                        count += 1
                        #print('Increase the count to',count)
                if count >= minsup:
                    #print(sequence,'is frequent!! Add it to the pruned list.\n\n')
                    fn_pruned += [sequence]
                    #print('Time to add it to the dictionary!')
                    fn_dict.append([count,sequence])
            else:
                for customer in customers:
                    #print('We are checking for',sequence,'in',customer)
                    test_sequence = [character for character in sequence]
                    found_in_customer = False
                    for transaction in customer:
                                                
                        if set(test_sequence).issubset(transaction):
                            #print(test_sequence,'is in',transaction)
                            if not found_in_customer:
                                found_in_customer = True
                                count += 1
                                #print('Increase the count to',count)
                            #print('Look for the rest of the items in',test_sequence)
                if count >= minsup:
                    #print(sequence,'is frequent!! Add it to the pruned list.\n\n')
                    fn_pruned += [sequence]
                    fn_dict.append([count,sequence])
        #print('The pruned list is:',fn_pruned)
        
        if not fn_pruned:
            done = True
            #print('WE DONE!')
        else:
            fk_dict = list(fn_dict)
            fn_dict = []
            
            fk = fn_pruned
            fn = big_gen_seq(fk)
    #print(fk_dict)
        
    return fk_dict
    
    
# Minimum support is the first item in data
# Customers' data is in every other list in data
data = grab_input()

try:
    customers = data[1:]
    minsup = int(data[0][0][0])
    
    f1_dict = frequent_1_items(customers, minsup)
    
    output = GSP(customers, f1_dict, minsup)

    for item in output:
        values = '['
        
        for word in item[1]:
            values += word + ' '
            
        values = values[:-1] + ']'
        print(item[0],values)
    
    #print(customers)
except:
    pass
