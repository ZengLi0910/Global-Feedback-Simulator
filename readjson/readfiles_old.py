def readentry(dictin,entry):
#function the recursive read and evaluate the entries in a dictionaty

#inputs
    #dictin : dicitionary that is being used
    #entry : the dicitionary value you would like

#outputs
    #value of entry is return as float if is can be evaluated using the
    #       in the dicitonary and otherwise it returns the string

    try:
        out=float(entry) # check if the entery of interest is already a  number
        
    except:
        try:
            #print'eval'
            out=eval(entry) #since it is not a number try to evaluate it
        except NameError as e: #if the entry can not be evaluated look at error to get missing entry 
            name=str(e).split("'")[1] #pull out the missing variable from the expressio
            #print 'name here'
            #print e
           # print name
            try:
                globals()[name]=readentry(dictin,dictin[str(name)]) #search search the dicitionary for the entry and add to local variables
                #locals()[name]=readentry(dictin,dictin[str(name)]) #search search the dicitionary for the entry and add to local variables
                #print 'below loc'
                #print name
                #print eval(name)
                #print locals()
                #out=eval(entry) #then evaluate if possible
                #out=readentry(dictin,name)
                out=readentry(dictin,entry)
            except:
                out = entry #if all else fails return the string in the entry
    #globals().clear()
        except TypeError:
            return entry
    return out 
